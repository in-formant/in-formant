#include "invglot.h"
#include "../../modules/math/constants.h"
#include "../filter/filter.h"
#include <cmath>
#include <iostream>

using namespace Analysis::Invglot;
using Analysis::InvglotResult;

IAIF::IAIF(double d)
    : d(d)
{
    lpc = std::make_unique<LP::Burg>();
}

static void applyWindow(const std::vector<double>& w, const std::vector<double>& x, std::vector<double>& y)
{
    y.resize(x.size());
    for (int i = 0; i < x.size(); ++i) {
        y[i] = x[i] * w[i];
    }
}

static std::vector<double> calculateLPC(const std::vector<double>& x, const std::vector<double>& w, int len, int order, std::unique_ptr<Analysis::LinpredSolver>& lpc)
{
    static std::vector<double> lpcIn;
    static double gain;

    lpcIn.resize(len);
    for (int i = 0; i < len; ++i) {
        lpcIn[i] = w[i] * x[x.size() / 2 - len / 2 + i];
    }
    auto a = lpc->solve(lpcIn.data(), len, order, &gain);
    a.insert(a.begin(), 1.0f);
    return a;
}

static std::vector<double> removePreRamp(const std::vector<double>& x, int preflt)
{
    return std::vector<double>(std::next(x.begin(), preflt), x.end());
}

inline double G(double x, int L, double alpha)
{
    const int N = L - 1;
    const double k = (x - N / 2.0f) / (2 * L * alpha);
    return expf(-(k * k));
}

static void calcGaussian(std::vector<double>& win, double alpha)
{
    const int L = win.size();
    
    double Gmh = G(-0.5f, L, alpha);
    double GmhpLpGmhmL = G(-0.5f + L, L, alpha) - G(-0.5f - L, L, alpha);

    for (int n = 0; n < L; ++n) {
        win[n] = G(n, L, alpha) - (Gmh * (G(n + L, L, alpha) + G(n - L, L, alpha))) / GmhpLpGmhmL;
    }
}

InvglotResult IAIF::solve(const double *xData, int length, double sampleRate)
{
    const int p_gl = 2 * std::round(sampleRate / 4000);
    const int p_vt = 2 * std::round(sampleRate / 2000) + 4;

    const int lpW = std::round(0.015f * sampleRate);

    std::vector<double> one({1.0f});
    std::vector<double> oneMinusD({1.0f, -d});

    static std::vector<double> window;
    if (window.size() != lpW) {
        window.resize(lpW);
        for (int i = 0; i < lpW; ++i) {
            window[i] = 0.5f - 0.5f * cosf((2.0f * M_PI * i) / (double) (length - 1));
        }
    }

    std::vector<double> x(xData, xData + length);
   
    static std::vector<std::array<double, 6>> hpfilt;
    if (hpfilt.empty()) {
        hpfilt = Analysis::butterworthHighpass(10, 70.0f, sampleRate);
    }

    int preflt = p_vt + 1;

    std::vector<double> xWithPreRamp(preflt + length);
    for (int i = 0; i < preflt; ++i) {
        xWithPreRamp[i] = 2.0f * ((double) i / (double) (preflt - 1) - 0.5f) * x[0];
    }
    for (int i = 0; i < length; ++i) {
        xWithPreRamp[preflt + i] = x[i];
    }

    xWithPreRamp = sosfilter(hpfilt, xWithPreRamp);
    x = removePreRamp(xWithPreRamp, preflt);

    auto Hg1 = calculateLPC(x, window, lpW, 1, lpc);
    auto y1 = removePreRamp(filter(Hg1, one, xWithPreRamp), preflt);

    auto Hvt1 = calculateLPC(y1, window, lpW, p_vt, lpc);
    auto g1 = removePreRamp(filter(one, oneMinusD, filter(Hvt1, one, xWithPreRamp)), preflt);

    auto Hg2 = calculateLPC(g1, window, lpW, p_gl, lpc);
    auto y = removePreRamp(filter(one, oneMinusD, filter(Hg2, one, xWithPreRamp)), preflt);

    auto Hvt2 = calculateLPC(y, window, lpW, p_vt, lpc);
    auto gWithPreRamp = filter(one, oneMinusD, filter(Hvt2, one, xWithPreRamp));

    auto g = removePreRamp(gWithPreRamp, preflt);

    double gMax = 1e-10;
    for (int i = 0; i < length; ++i) {
        double absG = fabs(g[i]);
        if (absG > gMax)
            gMax = absG;
    }
    for (int i = 0; i < length; ++i) {
        g[i] /= gMax;
    }

    return {
        .sampleRate = sampleRate,
        .glotSig = g,
    };
}
