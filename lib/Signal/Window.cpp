//
// Created by rika on 14/09/2019.
//

#include "Window.h"

using namespace Eigen;

static double bessel_i0(double x) {
    if (x < 0.0) return bessel_i0(- x);
    if (x < 3.75) {
        /* Formula 9.8.1. Accuracy 1.6e-7. */
        double t = x / 3.75;
        t *= t;
        return 1.0 + t * (3.5156229 + t * (3.0899424 + t * (1.2067492
                + t * (0.2659732 + t * (0.0360768 + t * 0.0045813)))));
    }
    /*
        otherwise: x >= 3.75
    */
    /* Formula 9.8.2. Accuracy of the polynomial factor 1.9e-7. */
    double t = 3.75 / x;   /* <= 1.0 */

    return exp (x) / sqrt (x) * (0.39894228 + t * (0.01328592
            + t * (0.00225319 + t * (-0.00157565 + t * (0.00916281
            + t * (-0.02057706 + t * (0.02635537 + t * (-0.01647633
            + t * 0.00392377))))))));
}

ArrayXd Window::createHamming(int L) {
    constexpr double a0 = 0.53836;
    constexpr double a1 = 0.46164;

    ArrayXd w(L);
    for (int k = 0; k < L; ++k) {
        w(k) = (a0 - a1 * cos((2.0 * M_PI * k) / static_cast<double>(L - 1)));
    }
    return w;
}

ArrayXd Window::createHanning(int L) {
    ArrayXd w(L);
    for (int k = 0; k < L; ++k) {
        w(k) = (0.5 - 0.5 * cos((2.0 * M_PI * k) / static_cast<double>(L - 1)));
    }
    return w;
}

ArrayXd Window::createBlackmanHarris(int L) {
    constexpr double a0 = 0.35875;
    constexpr double a1 = 0.48829;
    constexpr double a2 = 0.14128;
    constexpr double a3 = 0.01168;

    ArrayXd w(L);
    for (int k = 0; k < L; ++k) {
        w(k) = (a0
                - a1 * cos((2.0 * M_PI * k) / static_cast<double>(L - 1))
                + a2 * cos((4.0 * M_PI * k) / static_cast<double>(L - 1))
                - a3 * cos((6.0 * M_PI * k) / static_cast<double>(L - 1)));
    }
    return w;
}

ArrayXd Window::createGaussian(int L) {
    ArrayXd w(L);
    double imid = 0.5 * (L + 1), edge = std::exp(-12.0);
    for (int i = 1; i <= L; ++i) {
        w(i - 1) = (std::exp(-(48.0 * (i - imid) * (i - imid)) / (L + 1.0) / (L + 1.0)) - edge) / (1.0 - edge);
    }
    return w;
}

ArrayXd Window::createKaiser(int L) {
    constexpr double b = 0.5;
    const int N = L + 1;

    ArrayXd w(L);
    for (int k = 0; k < L; ++k) {
        w(k) = bessel_i0(b * sqrt(1 - ((k - N / 2.0) * (k - N / 2.0)) / ((N / 2.0) * (N / 2.0)))) / bessel_i0(b);
    }
    return w;
}