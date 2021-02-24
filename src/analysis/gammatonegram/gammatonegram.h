#ifndef ANALYSIS_GAMMATONEGRAM_H
#define ANALYSIS_GAMMATONEGRAM_H

#include "rpcxx.h"
#include <Eigen/Dense>

namespace Analysis {

    namespace ERB {
        constexpr double EarQ = 9.26449;
        constexpr double minBW = 24.7;
        constexpr double order = 1;
    }

    Eigen::ArrayXd erbSpace(double lowFreq, double highFreq, int N);

    rpm::vector<rpm::vector<std::array<double, 6>>> makeErbFilters(double fs, int numChannels, double lowFreq);

    Eigen::MatrixXd erbFilterBank(const rpm::vector<double>& x, const rpm::vector<rpm::vector<std::array<double, 6>>>& filters);

    Eigen::VectorXd gammatoneGram(const rpm::vector<double>& x, double fs, int N, double lowFreq);

}

#endif // ANALYSIS_GAMMATONEGRAM_H
