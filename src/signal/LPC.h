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

    enum Method {
        Auto = 0,
        Covar = 1,
        Burg = 2,
        Robust = 3,
    };

    void shortTermAnalysis(const Eigen::ArrayXd & sound, double windowDuration, double samplingFrequency, double timeStep, int * numberOfFrames, double * firstTime);

    Frames refineRobust(const Frames & lpc, const Eigen::ArrayXd & sound,
                        double analysisWidth, double preEmphasisFrequency,
                        double k_stdev, int itermax, double tol, bool wantLocation);

    Frames analyse(const Eigen::ArrayXd & sound, int predictionOrder,
                   double analysisWidth, double samplingFrequency,
                   double preEmphasisFrequency, int method);

    Frames analyseAuto(const Eigen::ArrayXd & sound, int predictionOrder,
                       double analysisWidth, double samplingFrequency,
                       double preEmphasisFrequency);

    Frames analyseCovar(const Eigen::ArrayXd & sound, int predictionOrder,
                       double analysisWidth, double samplingFrequency,
                       double preEmphasisFrequency);

    Frames analyseBurg(const Eigen::ArrayXd & sound, int predictionOrder,
                        double analysisWidth, double samplingFrequency,
                        double preEmphasisFrequency);


};

#endif //SPEECH_ANALYSIS_LPC_H
