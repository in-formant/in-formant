#include <functional>
#include <iostream>
#include "../Analyser.h"

static void smoothenPitch(const std::deque<double>& in, std::deque<double>& out);
static void smoothenFormants(const Formant::Frames& in, Formant::Frames& out);

void Analyser::applySmoothingFilters()
{
    smoothenPitch(pitchTrack, smoothedPitch);
    smoothenFormants(formantTrack, smoothedFormants);
}

void smoothenPitch(const std::deque<double>& in, std::deque<double>& out)
{
    constexpr double alpha = 0.8;

    static double prevOut = 130;

    out.pop_front();

    if (in.back() != 0) {
        prevOut += alpha * (in.back() - prevOut);
        out.push_back(prevOut);
    }
    else {
        out.push_back(0);
    }
}

void smoothenFormants(const Formant::Frames& in, Formant::Frames& out)
{
    constexpr int numForms = 4;
    constexpr double alpha = 0.5;
   
    static std::array<double, numForms> prevOut;

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
            prevOut[i] += alpha * (value - prevOut[i]);
            outValue = prevOut[i];
        }
        else {
            outValue = value;
        }
        
        outFrame.formant[i].frequency = outValue;
    }

    out.push_back(std::move(outFrame));
}


