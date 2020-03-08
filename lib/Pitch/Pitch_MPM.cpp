//
// Created by rika on 22/11/2019.
//

#include <cfloat>
#include "Pitch.h"
#include "McLeod/MPM.h"

using namespace Eigen;

constexpr double cutoff = 0.93;
constexpr double smallCutoff = 0.3;
constexpr double lowerPitchCutoff = 60.0;

void Pitch::estimate_MPM(const ArrayXd & x, double fs, Pitch::Estimation & result)
{
    ArrayXd nsdf = MPM::autocorrelation(x);
    nsdf /= nsdf.abs().maxCoeff();

    std::vector<int> maxPositions = MPM::peakPicking(nsdf);
    std::vector<std::pair<double, double>> estimates;

    double highestAmplitude = -DBL_MAX;

    for (int i : maxPositions) {
        highestAmplitude = std::max(highestAmplitude, nsdf(i));
        if (nsdf(i) > smallCutoff) {
            auto est = MPM::parabolicInterpolation(nsdf, i);
            estimates.push_back(est);
            highestAmplitude = std::max(highestAmplitude, std::get<1>(est));
        }
    }

    if (estimates.empty()) {
        result.pitch = 0;
        result.isVoiced = false;
        return;
    }

    double actualCutoff = cutoff * highestAmplitude;
    double pitch = 0;

    for (auto it = estimates.crbegin(); it != estimates.crend(); ++it) {
        if (std::get<1>(*it) >= actualCutoff) {
            pitch = fs / std::get<0>(*it);

            if (pitch < lowerPitchCutoff)
                continue;
            else
                break;
        }
    }

    if (pitch >= lowerPitchCutoff) {
        result.pitch = pitch;
        result.isVoiced = true;
    }
    else {
        result.pitch = 0;
        result.isVoiced = false;
    }
}
