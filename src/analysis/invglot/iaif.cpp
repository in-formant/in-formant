#include "invglot.h"
#include "../../modules/math/constants.h"
#include "../filter/filter.h"
#include <cmath>
#include <iostream>

using namespace Analysis::Invglot;
using Analysis::InvglotResult;

IAIF::IAIF(float d)
    : d(d)
{
    lpc = std::make_unique<LP::Burg>();
}

static void applyWindow(const std::vector<float>& w, const std::vector<float>& x, std::vector<float>& y)
{
    y.resize(x.size());
    for (int i = 0; i < x.size(); ++i) {
        y[i] = x[i] * w[i];
    }
}

static std::vector<float> calculateLPC(const std::vector<float>& x, const std::vector<float>& w, int len, int order, std::unique_ptr<Analysis::LinpredSolver>& lpc)
{
    static std::vector<float> lpcIn;
    static float gain;

    lpcIn.resize(len);
    for (int i = 0; i < len; ++i) {
        lpcIn[i] = w[i] * x[x.size() / 2 - len / 2 + i];
    }
    auto a = lpc->solve(lpcIn.data(), len, order, &gain);
    a.insert(a.begin(), 1.0f);
    return a;
}

static std::vector<float> removePreRamp(const std::vector<float>& x, int preflt)
{
    return std::vector<float>(std::next(x.begin(), preflt), x.end());
}

inline float G(float x, int L, float alpha)
{
    const int N = L - 1;
    const float k = (x - N / 2.0f) / (2 * L * alpha);
    return expf(-(k * k));
}

static void calcGaussian(std::vector<float>& win, float alpha)
{
    const int L = win.size();
    
    float Gmh = G(-0.5f, L, alpha);
    float GmhpLpGmhmL = G(-0.5f + L, L, alpha) - G(-0.5f - L, L, alpha);

    for (int n = 0; n < L; ++n) {
        win[n] = G(n, L, alpha) - (Gmh * (G(n + L, L, alpha) + G(n - L, L, alpha))) / GmhpLpGmhmL;
    }
}

InvglotResult IAIF::solve(const float *xData, int length, float sampleRate)
{
    const int p_gl = 2 * std::round(sampleRate / 4000);
    const int p_vt = 2 * std::round(sampleRate / 2000) + 4;

    const int lpW = std::round(0.015f * sampleRate);

    std::vector<float> one({1.0f});
    std::vector<float> oneMinusD({1.0f, -d});

    static std::vector<float> window;
    if (window.size() != lpW) {
        window.resize(lpW);
        for (int i = 0; i < lpW; ++i) {
            window[i] = 0.5f - 0.5f * cosf((2.0f * M_PI * i) / (float) (length - 1));
        }
    }

    int preflt = p_vt + 1;

    std::vector<float> x(xData, xData + length);
    
    std::vector<float> xWithPreRamp(preflt + length);
    for (int i = 0; i < preflt; ++i) {
        xWithPreRamp[i] = 2.0f * ((float) i / (float) (preflt - 1) - 0.5f) * x[0];
    }
    for (int i = 0; i < length; ++i) {
        xWithPreRamp[preflt + i] = x[i];
    }

    auto Hg1 = calculateLPC(x, window, lpW, 1, lpc);
    auto y1 = removePreRamp(filter(Hg1, one, xWithPreRamp), preflt);

    auto Hvt1 = calculateLPC(y1, window, lpW, p_vt, lpc);
    auto g1 = removePreRamp(filter(one, oneMinusD, filter(Hvt1, one, xWithPreRamp)), preflt);

    auto Hg2 = calculateLPC(g1, window, lpW, p_gl, lpc);
    auto y = removePreRamp(filter(one, oneMinusD, filter(Hg2, one, xWithPreRamp)), preflt);

    auto Hvt2 = calculateLPC(y, window, lpW, p_vt, lpc);
    auto gWithPreRamp = removePreRamp(filter(one, oneMinusD, filter(Hvt2, one, xWithPreRamp)), preflt);

    float hpAng = (2.0f * M_PI * 40.0f) / sampleRate;
    float lpAng = (2.0f * M_PI * 600.0f) / sampleRate;

    float hpFactor = expf(-hpAng);
    float lpFactor = lpAng / (lpAng + 1);
    
    std::vector<float> hpFilt {1.0f, -hpFactor};
    std::vector<float> lpFilt1 {lpFactor};
    std::vector<float> lpFilt2 {1.0f, (lpFactor - 1.0f)};

    auto g = removePreRamp(filter(lpFilt1, lpFilt2, filter(hpFilt, gWithPreRamp)), preflt);

    float gMax = 1e-10;
    for (int i = 0; i < g.size(); ++i) {
        float absG = fabsf(g[i]);
        if (absG > gMax)
            gMax = absG;
    }
    for (int i = 0; i < g.size(); ++i) {
        g[i] /= gMax;
    }

    return {
        .sampleRate = sampleRate,
        .glotSig = g,
    };
}
