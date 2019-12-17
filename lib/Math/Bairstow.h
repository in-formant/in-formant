//
// Created by clo on 15/11/2019.
//

#ifndef SPEECH_ANALYSIS_BAIRSTOW_H
#define SPEECH_ANALYSIS_BAIRSTOW_H

#include <Eigen/Core>
#include <vector>

namespace Bairstow {

    void solve(const Eigen::ArrayXd & poly, double initialMag, double initialPha,
               std::vector<Eigen::dcomplex> & roots,
               int maxTotalIter = 50, int maxIter = 200, double eps1 = 0.001, double eps2 = 0.0001);

}

#endif //SPEECH_ANALYSIS_BAIRSTOW_H
