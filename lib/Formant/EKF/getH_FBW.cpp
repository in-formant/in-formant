#include "EKF.h"

using namespace Eigen;

MatrixXd EKF::getH_FBW(
        Ref<const VectorXd> frmVals,
        Ref<const VectorXd> bwVals,
        const int numFormants,
        const int cepOrder,
        const double fs)
{
    MatrixXd H(cepOrder, 2 * numFormants);

    for (int i = 0; i < cepOrder; ++i) {
        for (int j = 0; j < numFormants; ++j) {
            const double bw_term = exp((-M_PI * (i + 1) * bwVals(j)) / fs);

            H(i, j) = -4 * M_PI / fs * bw_term * sin((2 * M_PI * (i + 1) * frmVals(j)) / fs);
            H(i, numFormants + j) = -2 * M_PI / fs * bw_term * cos((2 * M_PI * (i + 1) * frmVals(j)) / fs);
        }
    }

    return std::move(H);
}
