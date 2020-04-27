#include "df.h"
#include "../rpmalloc.h"
#include "../FFT/FFT.h"
#include "../LPC/LPC.h"
#include "../LPC/Frame/LPC_Frame.h"
#include "../Signal/Filter.h"

using namespace Eigen;

static ArrayXd dct(const ArrayXd& x, int trunc)
{
    const int N = x.size();
    
    /*ArrayXd out(trunc);

    for (int k = 0; k < trunc; ++k) {
        out(k) = 0.0;
        for (int n = 0; n < N; ++n) {
            out(k) += x(n) * cos((M_PI * (n + 0.5) * k) / (double) N);
        }
        out(k) *= sqrt(2.0 / (double) N);
        
        if (k == 0) {
            out(k) /= sqrt(2.0);
        }
    }

    return out;*/

    dct_plan(N);
    Map<ArrayXd>(dct_in(N), N) = x;
    dct(N);
    
    ArrayXd out = Map<ArrayXd>(dct_out(N), N).head(trunc);
    out *= sqrt(2.0 / (double) N);
    out(0) *= 1.0 / sqrt(2.0);

    return out;
} 

ArrayXd feature_periodogram(const ArrayXd& x, int nfft)
{
    rcfft_plan(nfft);

    Map<ArrayXd> in(rcfft_in(nfft), nfft);
   
    if (x.size() < nfft) {
        in.setZero();
        in.segment(nfft / 2 - x.size() / 2, x.size()) = x;
    }
    else {
        in = x.head(nfft);
    }

    rcfft(nfft);

    Map<ArrayXcd> out(rcfft_out(nfft), nfft / 2 + 1);
    ArrayXd pxx = real(conj(out) * out) / (double) x.size();

    return pxx;
}

ArrayXd feature_arspec(const ArrayXd& x, int order, int nfft)
{
    LPC::Frame lpc;
    lpc.nCoefficients = order;
    LPC::frame_burg(x, lpc);

    rcfft_plan(nfft);

    Map<ArrayXd> in(rcfft_in(nfft), nfft);
   
    in.setZero();
    in(0) = 1.0;
    in.segment(1, order) = lpc.a;
    
    rcfft(nfft);

    Map<ArrayXcd> out(rcfft_out(nfft), nfft / 2 + 1);
    ArrayXd pxx = (double) (2 << 17) / real(conj(out) * out);

    return pxx;
}

static constexpr int ncep_ar = 30;
static constexpr int ncep_ps = 50;

static constexpr int nfft_ar = 4096;
static constexpr int nfft_ps = 4096;

static constexpr double epsilon = 1e-10;

ArrayXd arSpecs(const ArrayXd& x, int order)
{
    static constexpr int nfft = nfft_ar;
    static constexpr int pn = nfft / 2 + 1;

    static ArrayXd freqs;
    if (freqs.size() != pn) {
        freqs.setLinSpaced(pn, 0, 0.5);
    }

    ArrayXd ars = feature_arspec(x, order, nfft);
    ArrayXd ar(pn);

    for (int i = 0; i < pn; ++i) {
        ar(i) = log(sqrt(ars(i) * ars(i) + freqs(i) * freqs(i)));

        if (ar(i) < 0)
            ar(i) = NAN;
        else if (ar(i) == 0)
            ar(i) = epsilon;
    }
   
    return dct(log10(ar), ncep_ar);
}

ArrayXd specPS(const ArrayXd& x, int pitch)
{
    static constexpr int nfft = nfft_ps;
    static constexpr int pn = nfft / 2 + 1;

    static ArrayXd freqs;
    if (freqs.size() != pn) {
        freqs.setLinSpaced(pn, 0, 0.5);
    }

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
        peri(i) = log(sqrt(specs(i) * specs(i) + freqs(i) * freqs(i)));

        if (peri(i) < 0)
            peri(i) = NAN;
        else if (peri(i) == 0)
            peri(i) = epsilon;
    }
   
    return dct(log10(peri), ncep_ps);
}

ArrayXd build_feature_row(const ArrayXd& x)
{
    const rpm::vector<int> lpcs{8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    
    // The feature things expect data in 16bit signed int format (
    ArrayXd input = x * (double) (2 << 17);
    Filter::preEmphasis(input, 50, 16000);
    
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
