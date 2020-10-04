#include "synthesis.h"
#include "../modules/math/constants.h"
#include <cmath>
#include <complex>

static std::vector<double> poly(const std::vector<std::complex<double>>& z)
{
    if (z.size() == 0) {
        return { 1.0 };
    }
    
    std::vector<std::complex<double>> p(z.size() + 1);

    p[0] = -z[0];
    p[1] = 1.0;

    for (int i = 1; i < z.size(); ++i) {
        for (int j = i + 1; j >= 1; --j) {
            p[j] = p[j - 1] - z[i] * p[j];
        }
        p[0] = -z[i] * p[0];
    }

    std::vector<double> pr(p.size());
    for (int i = 0; i < p.size(); ++i) {
        pr[i] = p[i].real();
    }

    return pr;
}

std::pair<std::vector<float>, std::vector<float>>
Synthesis::generateVTFilter(const std::vector<Analysis::FormantData>& formants,
                            const float Fs)
{
    std::vector<std::complex<double>> poles(formants.size() * 2);

    for (int i = 0; i < formants.size(); ++i) {
        const double r = exp(-M_PI * formants[i].bandwidth / Fs);
        const double phi = 2.0 * M_PI * formants[i].frequency / Fs;
    
        poles[i]                   = std::polar(r, phi);
        poles[formants.size() + i] = std::conj(poles[i]);
    }

    std::vector<double> B {0.1};
    std::vector<double> A = poly(poles);

    return {std::vector<float>(B.begin(), B.end()),
            std::vector<float>(A.rbegin(), A.rend())};
}
