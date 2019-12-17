//
// Created by clo on 03/12/2019.
//

#include <cmath>
#include "features.h"
#include "../../FFT/FFT.h"

using namespace Eigen;

constexpr double epsilon = 1e-9;

void buildFeatureRow(const ArrayXd & x, std::vector<double> & v)
{
    constexpr std::array<int, 10> lpcs{
        8, 9, 10, 11, 12, 13, 14, 15, 16, 17
    };

    ArrayXd vv;

    v.reserve(30 + 50 * lpcs.size());

    featureSpec(x, vv);
    v.insert(v.end(), vv.begin(), vv.end());

    for (auto j : lpcs) {
        featureArspec(x, j, vv);
        v.insert(v.end(), vv.begin(), vv.end());
    }

    // Replace NaNs with zeroes.
    std::replace_if(v.begin(), v.end(),
            [](double x) { return x != x; }, 0.0);

}

void featureSpec(const ArrayXd & x, ArrayXd & v)
{
    feat_spec specs;
    periodogram(x, specs, 4096);

    const int N = specs.pxx.size();
    ArrayXd peri(N);
    for (int n = 0; n < N; ++n) {
        double k = specs.pxx(n);
        double l = specs.fx(n);
        if (k == 0.0 && l == 0.0) {
            peri(n) = log10(epsilon);
        } else {
            peri(n) = log10(log(sqrt((k * k) + (l * l))));
        }
    }

    ArrayXd ceps;
    dct(peri, ceps);

    v = ceps.head<50>();
}

void featureArspec(const ArrayXd & x, int order, ArrayXd & v)
{
    feat_spec ars;
    arspec(x, ars, order, 4096);

    ArrayXd ar = (ars.pxx.square() + ars.fx.square()).sqrt().log();
    ar = (ar != 0.0).select(ar, epsilon).log10();

    ArrayXd ceps;
    dct(ar, ceps);

    v = ceps.head<30>();
}

void dct(const ArrayXd & in, ArrayXd & out)
{
    const int N = in.size();

    out.resize(N);
    for (int k = 0; k < N; ++k) {

        out(k) = 0.0;
        for (int n = 0; n < N; ++n) {
            out(k) += in(n) * cos((M_PI * k * (2 * n + 1)) / static_cast<double>(2 * N));
        }

        if (k == 0) {
            out(k) *= sqrt(1.0 / static_cast<double>(4 * N));
        } else {
            out(k) *= sqrt(1.0 / static_cast<double>(2 * N));
        }

        out(k) *= 2.0;
    }
}

void periodogram(const ArrayXd & x, feat_spec & r, int nfft)
{
    const int n = x.size();

    assert(n <= nfft);

    rcfft_plan(nfft);
    Map<ArrayXd> in(rcfft_in(nfft), nfft);
    in.setZero();
    in.head(n) = x;
    rcfft(nfft);
    Map<ArrayXcd> out(rcfft_out(nfft), nfft);

    const int pn = (nfft % 2 == 0) ? (nfft / 2 + 1) : ((nfft + 1) / 2);

    r.pxx = out.head(pn).abs().square() / static_cast<double>(n);
    r.fx.setLinSpaced(pn, 0, 0.5);
}

void arspec(const ArrayXd & x, feat_spec & r, int order, int nfft)
{
    ArrayXd a;
    double e;
    lpc_lev(x, order, a, e);

    const int pn = (nfft % 2 == 0) ? (nfft / 2 + 1) : ((nfft + 1) / 2);

    rcfft_plan(nfft);
    Map<ArrayXd> in(rcfft_in(nfft), nfft);
    in.setZero();
    in.head(order + 1) = a;
    rcfft(nfft);
    Map<ArrayXcd> out(rcfft_out(nfft), nfft);

    ArrayXcd px = 1.0 / out.head(pn);
    r.pxx = (conj(px) * px).real() * e;
    r.fx.setLinSpaced(pn, 0, 0.5);
}

void lpc_lev(const ArrayXd & x, int order, ArrayXd & a, double & e)
{
    const int maxlag = x.size();
    const int nfft = nextpow2(std::pow(2, 2 * maxlag - 1));
    rfft_plan(nfft);
    Map<ArrayXd> in(rfft_in(nfft), nfft);
    in.setZero();
    in.head(maxlag) = x;
    rfft(nfft);
    Map<ArrayXd> out(rfft_out(nfft), nfft);
    ArrayXd r = out.head(maxlag).abs().square();

    int i, j;
    double acc;
    a.setZero(order + 1);
    ArrayXd k(order);
    ArrayXd tmp(order);

    a(0) = 1.0;
    e = r(0);

    for (i = 1; i <= order; ++i) {
        acc = r(i);
        for (j = 1; j <= i - 1; ++j) {
            acc += a(j) * r(i - j);
        }
        k(i - 1) = -acc / e;
        a(i) = k(i - 1);

        tmp = a;

        for (j = 1; j < i; ++j) {
            a(j) += k(i - 1) * tmp(i - j);
        }
        e *= (1 - k(i - 1) * k(i - 1));
    }
}