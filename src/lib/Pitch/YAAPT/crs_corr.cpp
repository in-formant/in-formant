//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

using namespace Eigen;

void YAAPT::crs_corr(ConstRefXd data, int lagMin, int lagMax, RefXd phi)
{
    double eps1 = 0.0;
    int len = data.size();
    int N = len - lagMax;

    phi.setZero();

    // Remove DC level
    ArrayXd x = data - data.mean();

    VectorXd x_j = data.head(N);
    double p = x_j.dot(x_j);

    for (int k = lagMin; k <= lagMax; ++k) {
        // To calculate the dot product of the signal and the displaced version.
        VectorXd x_jr = data.segment(k, N);
        double num = x_j.dot(x_jr);

        // The normalization factor for the denominator.
        double q = x_jr.dot(x_jr);
        double den = p * q + eps1;

        phi(k) = num / sqrt(den);
    }

}