#include "YIN.h"

using namespace Eigen;

ArrayXd YIN::cumulative_mean_normalized_difference(const ArrayXd & x)
{
    double runningSum = 0;

    ArrayXd cmnd(x.size());

    cmnd(0) = 1;

    for (int tau = 1; tau < x.size(); ++tau) {
        runningSum += x(tau);
        cmnd(tau) = (tau * x(tau)) / runningSum;
    }

    return std::move(cmnd);
}
