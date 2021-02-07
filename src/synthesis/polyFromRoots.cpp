#include "synthesis.h"

rpm::vector<double> Synthesis::createPolynomialFromRoots(const rpm::vector<std::complex<double>>& z)
{
    rpm::vector<std::complex<double>> poly(z.size() + 1, 0.0);

    poly[0] = 1.0;

    for (int j = 0; j < (int) z.size(); ++j) {
        std::complex<double> zj = z[j];

        for (int i = j; i >= 0; --i) {
            poly[i + 1] -= zj * poly[i];
        }
    }

    rpm::vector<double> polyr(poly.size());
    for (int i = 0; i < (int) poly.size(); ++i) {
        polyr[i] = poly[i].real();
    }
    return polyr;
}
