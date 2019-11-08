//
// Created by clo on 08/11/2019.
//

#ifndef SPEECH_ANALYSIS_FORMANT_H
#define SPEECH_ANALYSIS_FORMANT_H

#include <Eigen/Core>
#include <vector>

#include "../LPC/LPC.h"

namespace Formant
{
    struct Formant {
        double frequency;
        double bandwidth;
    };

    struct Frame {
        int nFormants;
        std::vector<Formant> formant;
        double intensity;
    };

    void sort(Frame & frm);

    void frameFromRoots(const Eigen::ArrayXcd & r, Frame & frm, double samplingFrequency, double margin);
}

#endif //SPEECH_ANALYSIS_FORMANT_H
