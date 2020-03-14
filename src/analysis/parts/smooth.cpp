#include <functional>
#include <iostream>
#include <deque>
#include "../Analyser.h"

// Lowpass filter (Bessel, 0.1 cutoff, order 10)

constexpr double filt_gain = 3.827578934e+03;

constexpr int filt_ma_ord = 10;
constexpr std::array<double, filt_ma_ord> filt_ar = {
    -2.0781475859,
    2.7565526141,
    -2.4655110199,
    1.6257197445,
    -0.7980752984,
    0.2918280153,
    -0.0775106550,
    0.0142006224,
    -0.0016096927,
    0.0000852955,
};

constexpr int filt_ar_ord = 10;
constexpr std::array<double, filt_ar_ord> filt_ma = {
    10,
    45,
    120,
    210,
    252,
    210,
    120,
    45,
    10,
    1,
};

static double filterNext(std::deque<double> &prevIn, std::deque<double> &prevOut, double x);

static void smoothenPitch(const std::deque<double>& in, std::deque<double>& out);
static void smoothenFormants(const Formant::Frames& in, Formant::Frames& out);

void Analyser::applySmoothingFilters()
{
    smoothenPitch(pitchTrack, smoothedPitch);
    smoothenFormants(formantTrack, smoothedFormants);
}

void smoothenPitch(const std::deque<double>& in, std::deque<double>& out)
{
    static std::deque<double> prevIn(filt_ma_ord);
    static std::deque<double> prevOut(filt_ar_ord);

    out.pop_front();

    const double x = in.back();

    if (x != 0) {
        out.push_back(filterNext(prevIn, prevOut, x));
    }
    else {
        out.push_back(0);
    }
}

void smoothenFormants(const Formant::Frames& in, Formant::Frames& out)
{
    constexpr int numForms = 4;
   
    static std::array<std::deque<double>, numForms> prevIn;
    static std::array<std::deque<double>, numForms> prevOut;
    static bool init = false;

    if (!init) {
        for (int i = 0; i < numForms; ++i) {
            prevIn[i].resize(filt_ma_ord);
            prevOut[i].resize(filt_ar_ord);
        }
        init = true;
    }

    out.pop_front();

    const Formant::Frame& inFrame = in.back();

    const int nFormants = inFrame.nFormants;

    Formant::Frame outFrame;
    outFrame.nFormants = nFormants;
    outFrame.formant.resize(nFormants);

    for (int i = 0; i < nFormants; ++i) {
        const double value = inFrame.formant[i].frequency;

        double outValue;

        if (i < numForms) {
            outValue = filterNext(prevIn[i], prevOut[i], value);
        }
        else {
            outValue = value;
        }
        
        outFrame.formant[i].frequency = outValue;
    }

    out.push_back(std::move(outFrame));
}

static double filterNext(std::deque<double> &prevIn, std::deque<double> &prevOut, double x)
{
    x /= filt_gain;

    double y = x;

    for (int i = 0; i < filt_ma_ord; ++i) {
        y += filt_ma[i] * prevIn[i];
    }

    for (int i = 0; i < filt_ar_ord; ++i) {
        y -= filt_ar[i] * prevOut[i];
    }

    prevIn.pop_back();
    prevIn.push_front(x);

    prevOut.pop_back();
    prevOut.push_front(y);

    return y;
}
