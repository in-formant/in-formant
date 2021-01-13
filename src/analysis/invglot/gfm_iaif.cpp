#include "invglot.h"
#include "../../modules/math/constants.h"
#include "../filter/filter.h"
#include <cmath>
#include <iostream>

using namespace Analysis::Invglot;
using Analysis::InvglotResult;

GFM_IAIF::GFM_IAIF(double d)
    : d(d)
{
    lpc = std::make_unique<LP::Burg>();
}

/*
static void applyWindow(const std::vector<double>& w, const std::vector<double>& x, std::vector<double>& y)
{
    y.resize(x.size());
    for (int i = 0; i < x.size(); ++i) {
        y[i] = x[i] * w[i];
    }
}
*/

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

/*
static void calcGaussian(std::vector<double>& win, double alpha)
{
    const int L = win.size();
    
    double Gmh = G(-0.5f, L, alpha);
    double GmhpLpGmhmL = G(-0.5f + L, L, alpha) - G(-0.5f - L, L, alpha);

    for (int n = 0; n < L; ++n) {
        win[n] = G(n, L, alpha) - (Gmh * (G(n + L, L, alpha) + G(n - L, L, alpha))) / GmhpLpGmhmL;
    }
}
*/

static std::vector<double> conv(const std::vector<double>& x, const std::vector<double>& y) {
    int lx = x.size();
    int ly = y.size();
    int lw = lx + ly - 1;
    std::vector<double> w(lw, 0.0f);
    for (int k = 0; k < lw; ++k) {
        int i = k;
        for (int j = 0; j < ly; ++j) {
            if (i >= 0 && i < lx) {
                w[k] += x[i] * y[j];
            }
            i--;
        }
    }
    return w;
}

InvglotResult GFM_IAIF::solve(const double *xData, int length, double sampleRate)
{
    const int ng = 3;
    const int nv = std::round(sampleRate / 1000);
    const int Lpf = nv + 1;

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

    std::vector<double> s_gvl(xData, xData + length);

    static std::vector<std::array<double, 6>> hpfilt;
    if (hpfilt.empty()) {
        hpfilt = Analysis::butterworthHighpass(10, 70.0f, sampleRate);
    }
    s_gvl = sosfilter(hpfilt, s_gvl);

    std::vector<double> x_gvl(Lpf + length);
    for (int i = 0; i < Lpf; ++i) {
        x_gvl[i] = 2.0f * ((double) i / (double) (Lpf - 1) - 0.5f) * s_gvl[0];
    }
    for (int i = 0; i < length; ++i) {
        x_gvl[Lpf + i] = s_gvl[i];
    }
   
    auto s_gv = filter(one, oneMinusD, s_gvl);
    auto x_gv = filter(one, oneMinusD, x_gvl);
    
    auto ag1 = calculateLPC(s_gv, window, lpW, 1, lpc);

    for (int i = 1; i < ng; ++i) {
        auto x_v1x = filter(ag1, x_gv);
        auto s_v1x = removePreRamp(x_v1x, Lpf);
        
        auto ag1x = calculateLPC(s_v1x, window, lpW, 1, lpc); 

        ag1 = conv(ag1, ag1x);
    }

    auto x_v1 = filter(ag1, x_gv);
    auto s_v1 = removePreRamp(x_v1, Lpf);
    auto av1 = calculateLPC(s_v1, window, lpW, nv, lpc);

    auto x_g1 = filter(av1, x_gv);
    auto s_g1 = removePreRamp(x_g1, Lpf);
    auto ag = calculateLPC(s_g1, window, lpW, ng, lpc);

    auto x_v = filter(ag, x_gv);
    auto s_v = removePreRamp(x_v, Lpf);
    auto av = calculateLPC(s_v, window, lpW, nv, lpc);

    auto g = removePreRamp(filter(av, x_gv), Lpf);

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
