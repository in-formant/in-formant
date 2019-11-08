//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "../LPC.h"
#include "LPC_Frame.h"

using namespace Eigen;

bool LPC::frame_covar(const ArrayXd & x, LPC::Frame & lpc) {

    const int n = x.size();
    const int m = lpc.nCoefficients;
    int i = 1;

    lpc.a.setZero(m);

    ArrayXd b = ArrayXd::Zero(m * (m + 1) / 2 + 1);
    ArrayXd grc = ArrayXd::Zero(m + 1);
    ArrayXd a = ArrayXd::Zero(m + 2);
    ArrayXd beta = ArrayXd::Zero(m + 1);
    ArrayXd cc = ArrayXd::Zero(m + 2);

    lpc.gain = 0.0;
    for (i = m + 1; i <= n; ++i) {
        lpc.gain += x(i - 1) * x(i - 1);
        cc(1) += x(i - 1) * x(i - 2);
        cc(2) += x(i - 2) * x(i - 2);
    }

    if (lpc.gain == 0.0) {
        i = 1;
        goto end;
    }

    b(1) = 1.0;
    beta(1) = cc(2);
    a(1) = 1.0;
    a(2) = grc(1) = -cc(1) / cc(2);

    lpc.gain += grc(1) * cc(1);

    for (i = 2; i <= m; ++i) {
        double s = 0.0;
        for (int j = 1; j <= i; ++j) {
            cc(i - j + 2) = cc(i - j + 1) + x(m - i) * x(m - 1 - i + j)
                                          - x(n - i) * x(n - 1 - i + j);
        }

        cc(1) = 0.0;
        for (int j = m + 1; j <= n; ++j) {
            cc(1) += x(j - i - 1) * x(j - 1);
        }

        b(i * (i + 1) / 2) = 1.0;
        for (int j = 1; j <= i - 1; ++j) {
            double gam = 0.0;
            if (beta(j) < 0.0)
                goto end;
            else if (beta(j) == 0.0)
                continue;

            for (int k = 1; k <= j; ++k) {
                gam += cc(k + 1) * b(j * (j - 1) / 2 + k);
            }
            gam /= beta(j);

            for (int k = 1; k <= j; ++k) {
                b(i * (i - 1) / 2 + k) -= gam * b(j * (j - 1) / 2 + k);
            }
        }

        beta(i) = 0.0;
        for (int j = 1; j <= i; ++j) {
            beta(i) += cc(j + 1) * b(i * (i - 1) / 2 + j);
        }
        if (beta(i) <= 0.0)
            goto end;

        for (int j = 1; j <= i; ++j) {
            s += cc(j) * a(j);
        }
        grc(i) = -s / beta(i);

        for (int j = 2; j <= i; ++j) {
            a(j) += grc(i) * b(i * (i - 1) / 2 + j - 1);
        }
        a(i + 1) = grc(i);
        s = grc(i) * grc(i) * beta(i);
        lpc.gain -= s;
        if (lpc.gain <= 0.0)
            goto end;
    }

    end:

    i--;
    for (int j = 1; j <= i; ++j) {
        lpc.a(j - 1) = a(j + 1);
    }

#ifdef LPC_DEBUG
    std::cout << "LPC covariance: (" << i << " / " << m << ")" << std::endl;
#endif

    return (i == m);

}