//
// Created by clo on 25/11/2019.
//

#ifndef SPEECH_ANALYSIS_MFCC_H
#define SPEECH_ANALYSIS_MFCC_H

#include <Eigen/Core>

template<typename T>
inline T hz2mel(T f) {
    return 2595.0 * log10(1.0 + f / 700.0);
}

template<typename T>
inline T mel2hz(T m) {
    return 700.0 * (pow(10.0, m / 2595.0) - 1.0);
}

namespace MFCC
{
    Eigen::ArrayXXd initDCT(int numCepstra, int numFilters);

    Eigen::ArrayXXd initFilterBank(double lowFreq, double highFreq, double fs,
                                   int numFilters, int numFftBins);

    Eigen::ArrayXd applyDCT(const Eigen::ArrayXXd & dct, const Eigen::ArrayXd & lmfbCoef);

    void mfccFilter(const Eigen::ArrayXd & poly, double fs,
                    int numCepstra, int numFilters,
                    double lowFreq, double highFreq,
                    Eigen::ArrayXd & mfcc);

    void mfccSignal(const Eigen::ArrayXd & sound, double fs,
                    int numCepstra, int numFilters,
                    double lowFreq, double highFreq,
                    Eigen::ArrayXd & mfcc);
}

#endif //SPEECH_ANALYSIS_MFCC_H
