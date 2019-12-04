//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_LPC_H
#define SPEECH_ANALYSIS_LPC_H

#include <Eigen/Core>
#include <deque>

// #define LPC_DEBUG

namespace LPC {

    struct Frame {
        int nCoefficients;
        Eigen::ArrayXd a;
        double gain;
    };

    struct Frames {
        int maxnCoefficients;
        std::deque<Frame> d_frames;
    };

    enum Method {
        Auto = 0,
        Covar = 1,
        Burg = 2,
        Robust = 3,
    };

    void shortTermAnalysis(const Eigen::ArrayXd & sound, double windowDuration, double samplingFrequency, double timeStep, int * numberOfFrames, double * firstTime);

    Frames refineRobust(const Frames & lpc1, const Eigen::ArrayXd & _sound,
                        double samplingFrequency, double analysisWidth, double preEmphasisFrequency,
                        double k_stdev, int itermax, double tol, bool wantLocation);

    Frames analyse(const Eigen::ArrayXd & _sound, int predictionOrder, double samplingFrequency,
                   int method);

    Frames analyseAuto(const Eigen::ArrayXd & sound, int predictionOrder, double samplingFrequency);
    Frames analyseCovar(const Eigen::ArrayXd & sound, int predictionOrder, double samplingFrequency);
    Frames analyseBurg(const Eigen::ArrayXd & sound, int predictionOrder, double samplingFrequency);

};

#endif //SPEECH_ANALYSIS_LPC_H
