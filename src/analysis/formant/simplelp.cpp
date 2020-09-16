#include "formant.h"
#include "../util/util.h"

using namespace Analysis::Formant;
using Analysis::FormantResult;

FormantResult SimpleLP::solve(const float *lpc, int lpcOrder, float sampleRate)
{
    std::vector<float> polynomial(lpcOrder + 1);
    polynomial[0] = 1.0f;
    std::copy(lpc, lpc + lpcOrder, std::next(polynomial.begin()));
    
    std::vector<std::complex<float>> roots = findRoots(polynomial);

    FormantResult result;

    for (const auto& z : roots) {
        if (z.imag() < 0) continue;

        float r = std::abs(z);
        float phi = std::arg(z);
        
        result.formants.push_back(calculateFormant(r, phi, sampleRate));
    }

    sortFormants(result.formants);

    return result;
}
