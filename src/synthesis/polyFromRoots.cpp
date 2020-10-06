#include "synthesis.h"

std::vector<float> Synthesis::createPolynomialFromRoots(const std::vector<std::complex<float>>& z)
{
    std::vector<std::complex<double>> poly(z.size() + 1, 0.0);

    poly[0] = 1.0;

    for (int j = 0; j < z.size(); ++j) {
        std::complex<double> zj = z[j];

        for (int i = j; i >= 0; --i) {
            poly[i + 1] -= zj * poly[i];
        }
    }

    std::vector<float> polyr(poly.size());
    for (int i = 0; i < poly.size(); ++i) {
        polyr[i] = poly[i].real();
    }
    return polyr;
}
