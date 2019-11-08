//
// Created by clo on 13/09/2019.
//

#include "Pitch.h"

using namespace Eigen;

static constexpr double minFrequency = 70;
static constexpr double maxFrequency = 600;
static constexpr double ratio = 5.0;
static constexpr double sensitivity = 0.1;

void Pitch::estimate_AMDF(const ArrayXd & x, double fs, Pitch::Estimation & result) {

    const int maxShift = x.size();

    const int maxPeriod = std::ceil(fs / minFrequency);
    const int minPeriod = std::floor(fs / maxFrequency);

    ArrayXd amd = ArrayXd::Zero(maxShift);

    for (int i = 0; i < maxShift; ++i) {

        const auto frame1 = x.segment(0, maxShift - i);
        const auto frame2 = x.segment(i, maxShift - i);

        amd(i) = (frame1 - frame2).abs().sum();
    }

    const auto validAmd = amd.segment(minPeriod, maxPeriod - minPeriod);

    double minVal = validAmd.minCoeff();
    double maxVal = validAmd.maxCoeff();

    const double cutoff = std::round((sensitivity * (maxVal - minVal)) + minVal);

    int j = minPeriod;
    while (j <= maxPeriod && amd(j) > cutoff) {
        j++;
    }

    const int searchLength = minPeriod / 2;

    int minPos = j;
    minVal = amd(j);

    int i = j;
    while (i < j + searchLength && i <= maxPeriod) {
        i++;

        double cur = amd(j);
        if (cur < minVal) {
            minPos = i;
            minVal = cur;
        }
    }

    result.probability = -1;
    result.isVoiced = (std::round(minVal * ratio) < maxVal);

    if (result.isVoiced) {
        result.pitch = fs / static_cast<double>(minPos);
    }
    else {
        result.pitch = NAN;
    }

}