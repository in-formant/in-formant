#include "formant.h"
#include "../util/util.h"

using namespace Analysis::Formant;
using Analysis::FormantResult;

FormantResult SimpleLP::solve(const double *lpc, int lpcOrder, double sampleRate)
{
    std::vector<double> polynomial(lpcOrder + 1);
    polynomial[0] = 1.0f;
    std::copy(lpc, lpc + lpcOrder, std::next(polynomial.begin()));
    
    std::vector<std::complex<double>> roots = findRoots(polynomial);

    FormantResult result;

    for (const auto& z : roots) {
        if (z.imag() < 0) continue;

        double r = std::abs(z);
        double phi = std::arg(z);

        if (r > 1.0f) {
            continue;
        }

        result.formants.push_back(calculateFormant(r, phi, sampleRate));
    }

    sortFormants(result.formants);

    return result;
}
