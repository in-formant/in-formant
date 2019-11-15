//
// Created by clo on 15/11/2019.
//

#ifndef SPEECH_ANALYSIS_BAIRSTOW_H
#define SPEECH_ANALYSIS_BAIRSTOW_H

#include <Eigen/Core>

namespace Bairstow {

    void solve(const Eigen::ArrayXd & poly, const Eigen::dcomplex & z0, const Eigen::dcomplex & z1,
               std::vector<Eigen::dcomplex> & roots,
               int maxTotalIter = 200, int maxIter = 20, double eps1 = 0.001, double eps2 = 0.0001);

}

#endif //SPEECH_ANALYSIS_BAIRSTOW_H
