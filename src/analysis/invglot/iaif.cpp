#include "invglot.h"
#include "../../modules/math/constants.h"
#include "../filter/filter.h"
#include <cmath>

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

static std::vector<float> calculateLPC(const std::vector<float>& x, const std::vector<float>& w, int order, std::unique_ptr<Analysis::LinpredSolver>& lpc)
{
    static std::vector<float> lpcIn;
    static float gain;

    const int nx = x.size();
    lpcIn.resize(nx);
    for (int i = 0; i < nx; ++i) {
        lpcIn[i] = w[i] * x[i];
    }
    return lpc->solve(lpcIn.data(), nx, order, &gain);
}

static std::vector<float> removePreRamp(const std::vector<float>& x, int preflt)
{
    return std::vector<float>(std::next(x.begin(), preflt), x.end());
}

InvglotResult IAIF::solve(const float *xData, int length, float sampleRate)
{
    const int p_gl = 2 * std::round(sampleRate / 4000);
    const int p_vt = 2 * std::round(sampleRate / 2000) + 4;

    std::vector<float> one({1.0f});
    std::vector<float> oneMinusD({1.0f, -d});

    std::vector<float> hann(length);
    for (int i = 0; i < length; ++i) {
        hann[i] = 0.5f - 0.5f * cos(2.0f * M_PI / (length - 1));
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
    
    std::vector<float> lpcIn;
    std::vector<float> filtOut;
    float lpcGain;

    auto Hg1 = calculateLPC(x, hann, 1, lpc);
    auto y1 = removePreRamp(filter(Hg1, one, xWithPreRamp), preflt);

    auto Hvt1 = calculateLPC(y1, hann, p_vt, lpc);
    auto g1 = removePreRamp(filter(one, oneMinusD, filter(Hvt1, one, xWithPreRamp)), preflt);

    auto Hg2 = calculateLPC(g1, hann, p_gl, lpc);
    auto y = removePreRamp(filter(one, oneMinusD, filter(Hg2, one, xWithPreRamp)), preflt);

    auto Hvt2 = calculateLPC(y, hann, p_vt, lpc);
    auto g = removePreRamp(filter(one, oneMinusD, filter(Hvt2, one, xWithPreRamp)), preflt);

    float gMax = 0;
    for (int i = 0; i < g.size(); ++i) {
        if (fabs(g[i]) > gMax)
            gMax = fabs(g[i]);
    }
    for (int i = 0; i < g.size(); ++i) {
        g[i] /= gMax;
    }

    return {
        .sampleRate = sampleRate,
        .glotSig = g,
    };
}
