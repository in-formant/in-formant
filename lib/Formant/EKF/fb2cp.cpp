#include "EKF.h"

using namespace Eigen;

VectorXd EKF::fb2cp(
        Ref<const VectorXd> F,
        Ref<const VectorXd> BW,
        const int numCoeffs,
        const double fs)
{
    const int lenF = F.size();

    VectorXd C_int(lenF);
    VectorXd cepCoeffs(numCoeffs);

    for (int i = 0; i < numCoeffs; ++i) {
        for (int p = 0; p < lenF; ++p) {
            const double bw_term = (2.0 / double(i + 1)) * exp((-M_PI * (i + 1) * BW(p)) / fs);
            C_int(p) = bw_term * cos((2 * M_PI * (i + 1) * F(p)) / fs);
        }
        cepCoeffs(i) = C_int.sum();
    }

    return std::move(cepCoeffs);
}
