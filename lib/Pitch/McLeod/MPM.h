//
// Created by rika on 22/11/2019.
//

#ifndef SPEECH_ANALYSIS_MPM_H
#define SPEECH_ANALYSIS_MPM_H

#include <Eigen/Core>
#include <utility>
#include "../../rpmalloc.h"

using Eigen::ArrayXd, Eigen::Ref;

namespace MPM {

    ArrayXd autocorrelation(Ref<const ArrayXd> x);

    rpm::vector<int> peakPicking(Ref<const ArrayXd> x);

    std::pair<double, double> parabolicInterpolation(Ref<const ArrayXd> array, int x);

}

#endif //SPEECH_ANALYSIS_MPM_H
