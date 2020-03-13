#include "YIN.h"

using namespace Eigen;

int YIN::absolute_threshold(const ArrayXd & x, double threshold)
{
    int N = x.size();
    int tau;
    for (tau = 2; tau < N; ++tau) {
        if (x(tau) < threshold) {
            while (tau + 1 < N && x(tau + 1) < x(tau)) {
                tau++;
            }
            break;
        }
    }
    return (tau == N || x(tau) >= threshold) ? -1 : tau;
}
