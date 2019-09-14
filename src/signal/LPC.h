//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_LPC_H
#define SPEECH_ANALYSIS_LPC_H

#include <Eigen/Core>

namespace LPC {

    double analyse(const Eigen::ArrayXd & x, int order, Eigen::ArrayXd & a);

};

#endif //SPEECH_ANALYSIS_LPC_H
