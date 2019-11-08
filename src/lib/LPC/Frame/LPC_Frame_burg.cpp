//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "../LPC.h"
#include "LPC_Frame.h"

using namespace Eigen;

static double vecBurg(ArrayXd & a, const ArrayXd & x);

bool LPC::frame_burg(const ArrayXd & x, LPC::Frame & lpc) {

    lpc.a.resize(lpc.nCoefficients);

    lpc.gain = vecBurg(lpc.a, x);
    lpc.gain *= x.size();
    for (int i = 0; i < lpc.nCoefficients; ++i) {
        lpc.a(i) = -lpc.a(i);
    }

    return lpc.gain != 0.0;

}

double vecBurg(ArrayXd & a, const ArrayXd & x)
{
    int n = x.size(), m = a.size();
    a.setZero();

    ArrayXd b1 = ArrayXd::Zero(n + 1);
    ArrayXd b2 = ArrayXd::Zero(n + 1);
    ArrayXd aa = ArrayXd::Zero(m + 1);

    double p = 0.0;
    for (int j = 0; j < n; ++j)
        p += x(j) * x(j);

    double xms = p / static_cast<double>(n);
    if (xms <= 0.0)
        return xms;

    b1(1) = x(0);
    b2(n - 1) = x(n - 1);
    for (int j = 2; j <= n; ++j)
        b1(j) = b2(j - 1) = x(j - 1);

    for (int i = 1; i <= m; ++i) {
        double num = 0.0, denom = 0.0;
        for (int j = 1; j <= n - i; ++j) {
            num += b1(j) * b2(j);
            denom += b1(j) * b1(j) + b2(j) * b2(j);
        }

        if (denom <= 0.0)
            return 0.0;

        a(i - 1) = (2.0 * num) / denom;

        xms *= 1.0 - a(i - 1) * a(i - 1);

        for (int j = 1; j <= i - 1; ++j)
            a(j - 1) = aa(j) - a(i - 1) * aa(i - j);

        if (i < m) {
            for (int j = 1; j <= i; ++j)
                aa(j) = a(j - 1);
            for (int j = 1; j <= n - i - 1; ++j) {
                b1(j) -= aa(i) * b2(j);
                b2(j) = b2(j + 1) - aa(i) * b1(j + 1);
            }
        }
    }

    return xms;
}