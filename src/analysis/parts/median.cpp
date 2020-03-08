#include <functional>
#include <iostream>
#include "MedianFilter.hpp"
#include "../Analyser.h"

static void smoothenPitch(const std::deque<double>& in, std::deque<double>& out);
static void smoothenFormants(const Formant::Frames& in, Formant::Frames& out);

void Analyser::applyMedianFilters()
{
    smoothenPitch(pitchTrack, smoothedPitch);
    smoothenFormants(formantTrack, smoothedFormants);
}

void smoothenPitch(const std::deque<double>& in, std::deque<double>& out)
{
    constexpr int order = 5;
    
    static MedianFilter<double, order> filt;

    out.pop_front();

    if (in.back() != 0) {
        out.push_back(filt.Insert(in.back()));
    }
    else {
        out.push_back(0);
    }
}

void smoothenFormants(const Formant::Frames& in, Formant::Frames& out)
{
    constexpr int order = 11;
    
    static MedianFilter<double, order> filt1;
    static MedianFilter<double, order> filt2;
    static MedianFilter<double, order> filt3;
    
    out.pop_front();

    const Formant::Frame& inFrame = in.back();

    const int nFormants = inFrame.nFormants;

    Formant::Frame outFrame;
    outFrame.nFormants = nFormants;
    outFrame.formant.resize(nFormants);

    for (int i = 0; i < nFormants; ++i) {
        switch (i) {
        case 0:
            outFrame.formant[i].frequency = filt1.Insert(inFrame.formant[i].frequency);
            break;
        case 1:
            outFrame.formant[i].frequency = filt2.Insert(inFrame.formant[i].frequency);
            break;
        case 2:
            outFrame.formant[i].frequency = filt3.Insert(inFrame.formant[i].frequency);
            break;
        default:
            outFrame.formant[i].frequency = inFrame.formant[i].frequency;
            break;
        }
    }

    out.push_back(std::move(outFrame));
}


