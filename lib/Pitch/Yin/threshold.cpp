#include "YIN.h"

using namespace Eigen;

int YIN::absolute_threshold(const ArrayXd & x, double threshold)
{
    const int halfN = x.size();

    int tau;

    for (tau = 2; tau < halfN; ++tau) {
        if (x(tau) < threshold) {
            while (tau + 1 < halfN && x(tau + 1) < x(tau)) {
                tau++;
            }
            break;
        }
    }

    return (tau == halfN || x(tau) >= threshold) ? -1 : tau;
}
