#include "util.h"
#include <climits>

void Analysis::polishComplexRoot(const std::vector<float>& p, std::complex<float> *root, int maxIt)
{
    std::complex<float> zbest = *root;
    float ymin = HUGE_VALF;

    for (int iter = 1; iter <= maxIt; ++iter) {
        std::complex<float> y, dy;
        evaluatePolynomialWithDerivative(p, *root, &y, &dy);
        const float fabsy = std::abs(y);
        if (fabsy > ymin || fabsf(fabsy - ymin) < 1e-6) {
            *root = zbest;
            return;
        }
        ymin = fabsy;
        zbest = *root;
        if (std::abs(dy) == 0.0f) {
            return;
        }
        std::complex<float> dz = y / dy;
        *root -= dz;
    }
}

void Analysis::polishRealRoot(const std::vector<float>& p, float *root, int maxIt)
{
    float xbest = *root;
    float ymin = HUGE_VALF;

    for (int iter = 1; iter <= maxIt; ++iter) {
        float y, dy;
        evaluatePolynomialWithDerivative(p, *root, &y, &dy);
        const float fabsy = fabsf(y);
        if (fabsy > ymin || fabsf(fabsy - ymin) < 1e-6) {
            *root = xbest;
            return;
        }
        ymin = fabsy;
        xbest = *root;
        if (fabsf(dy) == 0.0f) {
            return;
        }
        float dx = y / dy;
        *root -= dx;
    }
}

void Analysis::polishRoots(const std::vector<float> &p, std::vector<std::complex<float>>& roots)
{
    constexpr int maxIt = 80;

    int i = 0;
    while (i < roots.size()) {
        float im = roots[i].imag();
        float re = roots[i].real();

        if (im != 0.0f) {
            polishComplexRoot(p, &roots[i], maxIt);
            if (i < roots.size() && im == -roots[i + 1].imag() && re == roots[i + 1].real()) {
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
