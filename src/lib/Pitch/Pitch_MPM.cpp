//
// Created by rika on 22/11/2019.
//

#include <numeric>
#include "Pitch.h"
#include "McLeod/MPM.h"

using namespace Eigen;

constexpr double cutoff = 0.93;
constexpr double smallCutoff = 0.2;
constexpr double lowerPitchCutoff = 60.0;

void Pitch::estimate_MPM(const ArrayXd & x, double fs, Pitch::Estimation & result)
{
    ArrayXd nsdf = MPM::autocorrelation(x);
    nsdf /= nsdf.abs().maxCoeff();

    std::vector<int> maxPositions = MPM::peakPicking(nsdf);
    std::vector<std::pair<double, double>> estimates;

    double highestAmplitude = std::numeric_limits<double>::min();

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
    double period = 0;

    for (auto i : estimates) {
        if (std::get<1>(i) >= actualCutoff) {
            period = std::get<0>(i);
            break;
        }
    }

    double pitchEstimate = (fs / period);

    result.pitch = pitchEstimate;
    result.isVoiced = true;

    if (pitchEstimate >= lowerPitchCutoff) {
        result.pitch = pitchEstimate;
        result.isVoiced = true;
    }
    else {
        result.pitch = 0;
        result.isVoiced = false;
    }
}
