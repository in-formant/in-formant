//
// Created by rika on 22/11/2019.
//

#ifndef SPEECH_ANALYSIS_MPM_H
#define SPEECH_ANALYSIS_MPM_H

#include <Eigen/Core>
#include <vector>
#include <utility>

using Eigen::ArrayXd, Eigen::Ref;

namespace MPM {

    ArrayXd autocorrelation(Ref<const ArrayXd> x, int w = -1);

    std::vector<int> peakPicking(Ref<const ArrayXd> x);

    std::pair<double, double> parabolicInterpolation(Ref<const ArrayXd> array, int x);

}

#endif //SPEECH_ANALYSIS_MPM_H
