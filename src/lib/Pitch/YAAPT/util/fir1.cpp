//
// Created by rika on 10/11/2019.
//

#include <iostream>
#include "../YAAPT.h"
#include "../../../Math/Polynomial.h"

using namespace Eigen;

void YAAPT::fir1(int n, const Eigen::ArrayXd & w, Eigen::ArrayXd & b)
{
    constexpr bool scale = true;
    int ftype = (w.size() == 1);

    // Build response function according to fir2 requirements.
    int bands = w.size() + 1;

    ArrayXd f = ArrayXd::Zero(2 * bands);
    f(0) = 0;
    f(2 * bands - 1) = 1;
    f(seq(1, 2 * bands - 2, 2)) = w;
    f(seq(2, 2 * bands - 2, 2)) = w;

    ArrayXd m = ArrayXd::Zero(2 * bands);
    m(seq(0, last, 2)) = (ArrayXd::LinSpaced(bands, 1, bands) - (1 - ftype)).unaryExpr([](double x) { return fmod(x, 2); });
    m(seq(1, last, 2)) = m(seq(0, last, 2));

    // Compute the filter.
    fir2(n, f, m, 0, 2, b);

    // Normalize filter magnitude
    if (scale) {
        double w_o;
        // Find the middle of the first band edge
        // Find the frequency of the normalizing gain
        if (m(0) == 1) {
            // If the first band is a passband, use DC gain
            w_o = 0;
        }
        else if (f(3) == 1) {
            // for a highpass filter,
            // use the gain at half the sample frequency
            w_o = 1;
        }
        else {
            // Otherwise, use the gain at the center
            // frequency of the first passband
            w_o = f(2) + (f(3) - f(2)) / 2.0;
        }

        // Compute |h(w_o)|^-1
        dcomplex h_wo;
        Polynomial::evaluate(b, std::polar(1.0, -M_PI * w_o), h_wo);
        double renorm = 1.0 / abs(h_wo);

        // Renormalize the filter
        b = renorm * b;
    }

}