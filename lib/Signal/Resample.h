//
// Created by rika on 11/10/2019.
//

#ifndef SPEECH_ANALYSIS_RESAMPLE_H
#define SPEECH_ANALYSIS_RESAMPLE_H

#include <Eigen/Core>

namespace Resample {

    enum {
        Nearest = 0,
        Linear = 1,
        Cubic = 2
    };

    Eigen::ArrayXd resample(const Eigen::ArrayXd & x, double sourceFs, double targetFs, int precision);

    Eigen::ArrayXd upsample(const Eigen::ArrayXd & x);

    double interpolate_sinc(const Eigen::ArrayXd & y, double x, int maxDepth);

}

#endif //SPEECH_ANALYSIS_RESAMPLE_H
