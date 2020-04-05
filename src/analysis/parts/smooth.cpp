#include <functional>
#include <iostream>
#include <deque>
//#include "MedianFilter.hpp"
#include "../Analyser.h"

// Lowpass filter (Bessel, 0.08 cutoff, order 6)

constexpr double filt_gain = 8.441780385e+02;

constexpr int filt_ma_ord = 6;
constexpr std::array<double, filt_ma_ord> filt_ar = {
    -2.5821171196,
    3.1008466477,
    -2.1439107533,
    0.8850057404,
    -0.2045193570,
    0.0205082331
};

constexpr int filt_ar_ord = 6;
constexpr std::array<double, filt_ar_ord> filt_ma = {
    6,
    15,
    20,
    15,
    6,
    1,
};

using IdxElt = std::pair<int, double>;

static void lfilt(std::vector<IdxElt> & data);
static void filtfilt(std::vector<IdxElt> & data);

static void smoothenPitch(const std::deque<double>& in, std::deque<double>& out);
static void smoothenFormants(const Formant::Frames& in, Formant::Frames& out);

void Analyser::applySmoothingFilters()
{
    //smoothenPitch(pitchTrack, smoothedPitch);
    smoothedPitch = pitchTrack;
   
    if (formantMethod == LP) {
        smoothenFormants(formantTrack, smoothedFormants);
    }
    else {
        smoothedFormants = formantTrack;
    }
}

void smoothenPitch(const std::deque<double>& in, std::deque<double>& out)
{
    static std::vector<IdxElt> input;

    input.clear();

    for (int i = 0; i < in.size(); ++i) {
        if (in[i] != 0) {
            input.push_back({i, in[i]});
        }
    }

    filtfilt(input);
    
    for (int i = 0; i < in.size(); ++i) {
        out[i] = 0.0;
    }

    for (const auto & [i, x] : input) {
        out[i] = x;
    }
}

void smoothenFormants(const Formant::Frames& in, Formant::Frames& out)
{
    constexpr int numForms = 3;
   
    static std::array<std::vector<IdxElt>, numForms> inputs;

    for (int k = 0; k < numForms; ++k) {
        inputs[k].clear();
    }

    out.resize(in.size());

    for (int i = 0; i < in.size(); ++i) {
        const int in_nFormants = in[i].nFormants;

        if (in_nFormants > numForms) {
            out[i].nFormants = in_nFormants;
            out[i].formant.resize(in_nFormants, {0});

            for (int k = numForms; k < in_nFormants; ++k) {
                out[i].formant[k].frequency = in[i].formant[k].frequency;
            }
        }
        else {
            out[i].nFormants = numForms;
            out[i].formant.resize(numForms, {0});
        }
        
        for (int k = 0; k < std::min(numForms, in_nFormants); ++k) {
            inputs[k].push_back({i, in[i].formant[k].frequency});
        }
    }

    for (int k = 0; k < numForms; ++k) {
        filtfilt(inputs[k]);
        
        for (const auto & [i, x] : inputs[k]) {
            out[i].formant[k].frequency = x;
        }
    }
}

static void filtfilt(std::vector<IdxElt> & data)
{
    if (data.size() == 0) return;

    lfilt(data);
    std::reverse(data.begin(), data.end());
    lfilt(data);
    std::reverse(data.begin(), data.end());
}

static void lfilt(std::vector<IdxElt> & data)
{
    constexpr int edge = 4 * std::max(filt_ma_ord, filt_ar_ord);
    std::vector<IdxElt> edgeData(edge);

    for (int i = 0; i < edge; ++i) {
        edgeData[i] = data.front();
    }
    data.insert(data.begin(), edgeData.begin(), edgeData.end());

    static std::deque<double> prevIn(filt_ma_ord);
    static std::deque<double> prevOut(filt_ar_ord);

    for (int i = 0; i < filt_ma_ord; ++i) {
        prevIn[i] = 0.0;
    }
    for (int i = 0; i < filt_ar_ord; ++i) {
        prevOut[i] = 0.0;
    }

    std::vector<IdxElt> out;

    for (const auto & [i, x] : data) {
        double y = x / filt_gain;

        for (int i = 0; i < filt_ma_ord; ++i) {
            y += filt_ma[i] * prevIn[i];
        }

        for (int i = 0; i < filt_ar_ord; ++i) {
            y -= filt_ar[i] * prevOut[i];
        }

        prevIn.pop_back();
        prevIn.push_front(x / filt_gain);

        prevOut.pop_back();
        prevOut.push_front(y);

        out.push_back({i, y});
    }

    data = std::move(out);
    data.erase(data.begin(), data.begin() + edge);
}
