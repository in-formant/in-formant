//
// Created by clo on 08/11/2019.
//

#ifndef SPEECH_ANALYSIS_FORMANT_H
#define SPEECH_ANALYSIS_FORMANT_H

#include <Eigen/Core>
#include <vector>
#include <deque>

#include "../LPC/LPC.h"

struct root {
    double r, phi;
    double f, b;
};

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

    using Frames = std::deque<Frame>;

    void sort(Frame & frm);

    void frameFromRoots(const Eigen::ArrayXd & p,
                        const Eigen::ArrayXcd & r, Frame & frm,
                        double samplingFrequency);

    double calculateVTL(const Frames & frames);

    bool track(std::deque<Frame> & frms,
               int ntrack,
               double refF1, double refF2, double refF3, double refF4, double refF5,
               double dfCost, double bfCost, double octaveJumpCost);
}

#endif //SPEECH_ANALYSIS_FORMANT_H
