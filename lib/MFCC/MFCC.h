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

#endif //SPEECH_ANALYSIS_MFCC_H
