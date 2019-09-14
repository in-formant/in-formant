//
// Created by clo on 12/09/2019.
//

#include "LPC.h"

using namespace Eigen;

double LPC::analyse(const ArrayXd &x, int order, ArrayXd &a) {

    a.setZero(order);

    double aut[order + 1];
    double error;
    double epsilon;
    int n = x.size();
    int i, j;

    /* autocorrelation, p+1 lag coefficients */
    j = order + 1;
    while (j--) {
        double d = 0; /* double needed for accumulator depth */
        for (i = j; i < n; i++)
            d += x(i) * x(i - j);

        aut[j] = d;
    }

    /* Generate lpc coefficients from autocorr values */

    /* set our noise floor to about -100dB */
    error = aut[0] * (1. + 1e-10);
    epsilon = 1e-9 * aut[0] + 1e-10;

    for (i = 0; i < order; i++) {
        double r = -aut[i + 1];

        if (error < epsilon) {
            for (j = i; j < order; j++)
                a(j) = 0;
            goto done;
        }

        /* Sum up this iteration's reflection coefficient; note that in
           Vorbis we don't save it.  If anyone wants to recycle this code
           and needs reflection coefficients, save the results of 'r' from
           each iteration. */

        for (j = 0; j < i; j++)
            r -= a(j) * aut[i - j];

        r /= error;

        /* Update LPC coefficients and total error */

        a(i) = r;

        for (j = 0; j < i / 2; j++) {
            double tmp = a(j);

            a(j) += r * a(i - 1 - j);
            a(i - 1 - j) += r * tmp;
        }

        if (i & 1)
            a(j) += a(j) * r;

        error *= 1. - r * r;

    }

    done:

    /* slightly damp the filter */
    double g = .99;
    double damp = g;
    for (j = 0; j < order; j++) {
        a(j) *= damp;
        damp *= g;
    }

    return error;

}