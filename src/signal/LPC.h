//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_LPC_H
#define SPEECH_ANALYSIS_LPC_H

#include <Eigen/Core>
#include <vector>

// #define LPC_DEBUG

namespace LPC {

    struct Frame {
        int nCoefficients;
        Eigen::ArrayXd a;
        double gain;
    };

    struct Frames {
        int maxnCoefficients;
        std::vector<Frame> d_frames;
    };

    void shortTermAnalysis(const Eigen::ArrayXd & sound, double windowDuration, double timeStep, int * numberOfFrames, double * firstTime);

    Frames robust(const Frames & lpc, const Eigen::ArrayXd & sound,
            double analysisWidth, double preEmphasisFrequency,
            double k_stdev, int itermax, double tol, bool wantLocation);

};

#endif //SPEECH_ANALYSIS_LPC_H
