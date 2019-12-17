//
// Created by clo on 08/11/2019.
//

#include "Polynomial.h"

using namespace Eigen;

void Polynomial::fixRootsIntoUnitCircle(ArrayXcd & r)
{
    for (int i = 0; i < r.size(); ++i) {
        if (std::abs(r(i)) > 1.0) {
            r(i) = 1.0 / std::conj(r(i));
        }
    }
}

void Polynomial::polishRoot(const Eigen::ArrayXd & poly, double * x, int maxit)
{
    constexpr double eps = std::numeric_limits<double>::epsilon();

    double xbest = *x;
    double pmin = 1e308;

    for (int i = 0; i < maxit; ++i) {
        double p, dp;
        Polynomial::evaluateWithDerivative(poly, *x, p, dp);
        double absp = std::abs(p);
        if (absp > pmin || std::abs(absp - pmin) < eps) {
            /* We stop because the approximation is getting worse.
               Return the previous (hitherto best) value for x. */
            *x = xbest;
            return;
        }
        pmin = absp;
        xbest = *x;
        if (std::abs(dp) == 0.0)
            return;
        // Newton-Raphson
        double dx = p / dp;
        *x -= dx;
    }
}

void Polynomial::polishRoot(const Eigen::ArrayXd & poly, dcomplex * z, int maxit)
{
    constexpr double eps = std::numeric_limits<double>::epsilon();

    dcomplex zbest = *z;
    double pmin = 1e308;

    for (int i = 0; i < maxit; ++i) {
        dcomplex p, dp;
        Polynomial::evaluateWithDerivative(poly, *z, p, dp);
        double absp = std::abs(p);
        if (absp > pmin || std::abs(absp - pmin) < eps) {
            /* We stop because the approximation is getting worse.
               Return the previous (hitherto best) value for x. */
            *z = zbest;
            return;
        }
        pmin = absp;
        zbest = *z;
        if (std::abs(dp) == 0.0)
            return;
        // Newton-Raphson
        dcomplex dz = p / dp;
        *z -= dz;
    }
}

void Polynomial::polishRoots(const Eigen::ArrayXd & p, Eigen::ArrayXcd & r)
{
    constexpr int maxit = 80;
    int i = 0;
    while (i < r.size()) {
        double re = r(i).real();
        double im = r(i).imag();

        if (im != 0.0) {
            Polynomial::polishRoot(p, &r(i), maxit);
            // Check for complex-conjugate pairs.
            if (i < r.size() && re == r(i + 1).real() && im == -r(i + 1).imag()) {
                r(i + 1) = std::conj(r(i));
                i++;
            }
        }
        else {
            Polynomial::polishRoot(p, &re, maxit);
            r(i) = re;
        }

        i++;
    }
}