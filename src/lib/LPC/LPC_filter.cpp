//
// Created by rika on 12/10/2019.
//

#include "LPC.h"
#include "Frame/LPC_Frame.h"

using namespace Eigen;

void LPC::filter(const LPC::Frame & lpc, ArrayXd & x)
{
    for (int i = 0; i < x.size(); ++i) {
        int m = (i >= lpc.nCoefficients ? lpc.nCoefficients - 1 : i);
        for (int j = 0; j <= m; ++j) {
            x(i) -= lpc.a(j) * x(i - j);
        }
    }
}

void LPC::filterInverse(const LPC::Frame & lpc, ArrayXd & x)
{
    ArrayXd y = ArrayXd::Zero(lpc.nCoefficients);
    for (int i = 0; i < x.size(); ++i) {
        double y0 = x(i);
        for (int j = 0; j < lpc.nCoefficients; ++j)
            x(i) += lpc.a(j) * y(j);
        for (int j = lpc.nCoefficients - 1; j > 0; --j)
            y(j) = y(j - 1);
        y(0) = y0;
    }
}