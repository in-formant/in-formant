#include "gammatonegram.h"

Eigen::ArrayXd Analysis::erbSpace(double lowFreq, double highFreq, int N)
{
    using namespace ERB;
    Eigen::ArrayXd cfArray(N);

    for (int n = 0; n < N; ++n) {
        cfArray(n) = -(EarQ * minBW) + exp((n + 1) * (-log(highFreq + EarQ * minBW) + log(lowFreq + EarQ * minBW)) / N) * (highFreq + EarQ * minBW);
    }
    return cfArray;
}
