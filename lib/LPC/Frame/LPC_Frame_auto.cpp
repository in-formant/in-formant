//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "../LPC.h"
#include "LPC_Frame.h"

using namespace Eigen;

int LPC::frame_auto(const ArrayXd & x, LPC::Frame & lpc) {

    const int n = x.size();
    const int m = lpc.nCoefficients;
    int i = 1;

    lpc.a.setZero(m);

    ArrayXd r = ArrayXd::Zero(m + 2);
    ArrayXd a = ArrayXd::Zero(m + 2);
    ArrayXd rc = ArrayXd::Zero(m + 1);

    for (i = 1; i <= m + 1; ++i) {
        for (int j = 1; j <= n - i + 1; ++j) {
            r(i) += x(j - 1) * x(j + i - 2);
        }
    }

    if (r(1) == 0.0) {
        i = 1;
        goto end;
    }

    a(1) = 1.0;
    a(2) = rc(1) = -r(2) / r(1);

    lpc.gain = r(1) + r(2) * rc(1);

    for (i = 2; i <= m; ++i) {
        double s = 0.0;
        for (int j = 1; j <= i; ++j) {
            s += r(i - j + 2) * a(j);
        }
        rc(i) = -s / lpc.gain;
        for (int j = 2; j <= i / 2 + 1; ++j) {
            double at = a(j) + rc(i) * a(i - j + 2);
            a(i - j + 2) += rc(i) * a(j);
            a(j) = at;
        }
        a(i + 1) = rc(i);

        lpc.gain += rc(i) * s;
        if (lpc.gain <= 0) {
            goto end;
        }
    }

    end:

    i--;
    for (int j = 1; j <= i; ++j) {
        lpc.a(j - 1) = a(j + 1);
    }

#ifdef LPC_DEBUG
    std::cout << "LPC autocorrelation: (" << i << " / " << m << ")" << std::endl;
#endif

    return i;
}
