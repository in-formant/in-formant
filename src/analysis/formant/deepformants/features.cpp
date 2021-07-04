#include "df.h"
#include "../../fft/fft.h"
#include "../../linpred/linpred.h"
#include <memory>

using namespace Eigen;

template<typename Derived>
static ArrayXd dct(const ArrayBase<Derived>& x, int trunc)
{
    const int N = x.size();
    
    auto dct = DFModelHolder::instance()->dct(N);

    for (int i = 0; i < N; ++i) {
        dct->data(i) = x(i);
    }
    dct->compute();

    ArrayXd out(trunc);
    out(0) = dct->data(0) / sqrt(4 * N);
    for (int i = 1; i < trunc; ++i) {
        out(i) = dct->data(i);
    }
    return out;
}

template<typename Derived>
static ArrayXd feature_periodogram(const ArrayBase<Derived>& x, int nfft)
{
    const int N = x.size();

    auto fft = DFModelHolder::instance()->fft1(nfft);
    
    if (N < nfft) {
        for (int i = 0; i < nfft; ++i)
            fft->input(i) = 0.0;
        for (int i = 0; i < N; ++i) 
            fft->input(nfft / 2 - N / 2 + i) = x(i);
    }
    else if (N > nfft) {
        for (int i = 0; i < nfft; ++i) {
            fft->input(i) = x(N / 2 - nfft / 2 + i);
        }
    }
    else {
        for (int i = 0; i < nfft; ++i) {
            fft->input(i) = x(i);
        }
    }

    fft->computeForward();

    int outLength = fft->getOutputLength();
    ArrayXd out(outLength);
    for (int i = 0; i < outLength; ++i) {
        double px = std::abs(fft->output(i));
        out(i) = (px * px) / N;
    }
    return out;
}

template<typename Derived>
static ArrayXd feature_arspec(const ArrayBase<Derived>& x, int order, int nfft)
{
    Analysis::LP::Autocorr lpc;
    double e;
    rpm::vector<double> xv(x.size());
    for (int i = 0; i < x.size(); ++i)
        xv[i] = x(i);
    auto a = lpc.solve(xv.data(), xv.size(), order, &e);

    auto fft = DFModelHolder::instance()->fft2(nfft);

    fft->input(0) = 1.0;
    for (int i = 1; i <= a.size(); ++i) 
        fft->input(i) = a[i - 1];
    for (int i = a.size() + 1; i < nfft; ++i)
        fft->input(i) = 0.0; 

    fft->computeForward();

    int outLength = fft->getOutputLength();
    ArrayXd out(outLength);
    for (int i = 0; i < outLength; ++i) {
        auto px = std::abs(1.0 / fft->output(i));
        out(i) = px * px * e;
    }
    return out;
}

template<typename Derived>
static ArrayXd atal(const ArrayBase<Derived>& x, int order, int numCoefs)
{
    Analysis::LP::Autocorr lpc;
    double e;
    rpm::vector<double> xv(x.size());
    for (int i = 0; i < x.size(); ++i)
        xv[i] = x(i);
    auto a = lpc.solve(xv.data(), xv.size(), order, &e);
    int m, k, p = a.size();

    ArrayXd c(numCoefs);
    c.setZero();
    
    c(0) = 1.0;
    for (m = 1; m < p + 1; ++m) {
        c(m) = -a[m - 1];
        for (k = 1; k < m; ++k)
            c(m) += (double(k) / double(m) - 1) * a[k - 1] * c(m - k);
    }
    for (m = p + 1; m < numCoefs; ++m) {
        c(m) = 0.0;
        for (k = 1; k < p + 1; ++k)
            c(m) += (double(k) / double(m) - 1) * a[k - 1] * c(m - k);
    }
    return c;
}

constexpr int ncep_ar = 30;
constexpr int ncep_ps = 50;

constexpr int nfft_ar = 4096;
constexpr int nfft_ps = 4096;

constexpr double epsilon = 1e-10;

template<typename Derived>
static ArrayXd arSpecs(const ArrayBase<Derived>& x, int order)
{
    //return atal(x, order, ncep_ar);

    constexpr int nfft = nfft_ar;
    constexpr int pn = nfft / 2 + 1;

    ArrayXd freqs = ArrayXd::LinSpaced(pn, 0, 0.5);

    ArrayXd ars = feature_arspec(x, order, nfft);
    ArrayXd ar(pn);

    for (int i = 0; i < pn; ++i) {
        ar(i) = log(sqrt(ars(i) * ars(i) + freqs(i) * freqs(i)));
        
        if (ar(i) < 0.0)
            ar(i) = NAN;
        else if (ar(i) == 0.0)
            ar(i) = epsilon;
    }
   
    return dct(log10(ar), ncep_ar);
}

template<typename Derived>
static ArrayXd specPS(const ArrayBase<Derived>& x, int pitch)
{
    constexpr int nfft = nfft_ps;
    constexpr int pn = nfft / 2 + 1;

    ArrayXd freqs = ArrayXd::LinSpaced(pn, 0, 0.5);

    int samps = x.size() / pitch;
    if (samps == 0)
        samps = 1;
    int frames = x.size() / samps;

    ArrayXd specs = feature_periodogram(x.head(frames), nfft);
    for (int i = 1; i < samps; ++i) {
        specs += feature_periodogram(x.segment(i * frames, frames), nfft);
    }
    specs /= (double) samps;

    ArrayXd peri(pn);

    for (int i = 0; i < pn; ++i) {
        peri(i) = sqrt(specs(i) * specs(i) + freqs(i) * freqs(i));
        if (peri(i) > 0.0)
            peri(i) = log(peri(i));
        else if (peri(i) == 0.0)
            peri(i) = epsilon;
        else
            peri(i) = NAN;
    }
   
    return dct(log10(peri), ncep_ps);
}

template<typename Derived>
ArrayXd build_feature_row(const ArrayBase<Derived>& x)
{
    const rpm::vector<int> lpcs{8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    
    // The feature things expect data in 16bit signed int format.
    ArrayXd input = x * MAX_AMPLITUDE_16BIT;
    
    ArrayXd arr(ncep_ps + lpcs.size() * ncep_ar);

    arr.head(ncep_ps) = specPS(input, 50);

    int start = ncep_ps;
    for (int order : lpcs) {
        arr.segment(start, ncep_ar) = arSpecs(input, order);
        start += ncep_ar;
    }

    for (int i = 0; i < arr.size(); ++i) {
        if (std::isnan(arr(i)))
            arr(i) = 0.0;
    }

    return arr;
}

using ImplT = Map<const ArrayXd>;
template ArrayXd build_feature_row<ImplT>(const ArrayBase<ImplT>& x);
