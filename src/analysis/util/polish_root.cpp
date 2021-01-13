#include "util.h"
#include <climits>

void Analysis::polishComplexRoot(const std::vector<double>& p, std::complex<double> *root, int maxIt)
{
    std::complex<double> zbest = *root;
    double ymin = HUGE_VALF;

    for (int iter = 1; iter <= maxIt; ++iter) {
        std::complex<double> y, dy;
        evaluatePolynomialWithDerivative(p, *root, &y, &dy);
        const double fabsy = std::abs(y);
        if (fabsy > ymin || fabs(fabsy - ymin) < 1e-6) {
            *root = zbest;
            return;
        }
        ymin = fabsy;
        zbest = *root;
        if (std::abs(dy) == 0.0f) {
            return;
        }
        std::complex<double> dz = y / dy;
        *root -= dz;
    }
}

void Analysis::polishRealRoot(const std::vector<double>& p, double *root, int maxIt)
{
    double xbest = *root;
    double ymin = HUGE_VALF;

    for (int iter = 1; iter <= maxIt; ++iter) {
        double y, dy;
        evaluatePolynomialWithDerivative(p, *root, &y, &dy);
        const double fabsy = fabs(y);
        if (fabsy > ymin || fabs(fabsy - ymin) < 1e-6) {
            *root = xbest;
            return;
        }
        ymin = fabsy;
        xbest = *root;
        if (fabs(dy) == 0.0f) {
            return;
        }
        double dx = y / dy;
        *root -= dx;
    }
}

void Analysis::polishRoots(const std::vector<double> &p, std::vector<std::complex<double>>& roots)
{
    constexpr int maxIt = 80;

    int i = 0;
    while (i < (int) roots.size()) {
        double im = roots[i].imag();
        double re = roots[i].real();

        if (im != 0.0f) {
            polishComplexRoot(p, &roots[i], maxIt);
            if (i < (int) roots.size() && im == -roots[i + 1].imag() && re == roots[i + 1].real()) {
                roots[i + 1] = std::conj(roots[i]);
                i++;
            }
        }
        else {
            polishRealRoot(p, &re, maxIt);
            roots[i] = re;
        }
        i++; 
    }
}
