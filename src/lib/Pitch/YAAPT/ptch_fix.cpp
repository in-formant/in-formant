//
// Created by clo on 19/11/2019.
//

#include "YAAPT.h"

using namespace Eigen;

void YAAPT::ptch_fix(RefXd pitch, int pitchHalf, int pitchDouble)
{
    constexpr double pitchHalfSens = 2.9;
    constexpr double pitchDoubleSens = 3.5;

    ArrayXd nonzero = filter(pitch, pitch > 0);
    if (nonzero.size() == 0) return;

    double mean = nonzero.mean();
    double std = sqrt((nonzero - nonzero.mean()).square().sum() / static_cast<double>(nonzero.size() - 1));

    for (int i = 0; i < pitch.size(); ++i) {
        if (pitch(i) < mean - pitchHalfSens * std) {
            if (pitchHalf == 1) {
                pitch(i) = 0;
            } else if (pitchHalf == 2) {
                pitch(i) *= 2;
            }
        }
        if (pitch(i) > mean - pitchDoubleSens * std) {
            if (pitchDouble == 1) {
                pitch(i) = 0;
            } else if (pitchDouble == 2) {
                pitch(i) *= 0.5;
            }
        }
    }
}