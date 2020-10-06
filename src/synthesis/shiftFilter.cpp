#include "../modules/math/constants.h"
#include "../analysis/analysis.h"
#include "synthesis.h"
#include <iostream>

static std::vector<float> conv(const std::vector<float>& x, const std::vector<float>& y) {
    int lx = x.size();
    int ly = y.size();
    int lw = lx + ly - 1;
    std::vector<float> w(lw, 0.0f);
    for (int k = 0; k < lw; ++k) {
        int i = k;
        for (int j = 0; j < ly; ++j) {
            if (i >= 0 && i < lx) {
                w[k] += x[i] * y[j];
            }
            i--;
        }
    }
    return w;
}

std::pair<std::vector<float>, float> Synthesis::frequencyShiftFilter(const std::vector<Analysis::FormantData>& formants, float Fs, float factor)
{
    const float freqMin = 50.0f;
    const float freqMax = Fs / 2.0f - 50.0f;
    
    const float melMin = 2595.0f * log10f(1.0f + freqMin / 700.0f);
    const float melMax = 2595.0f * log10f(1.0f + freqMax / 700.0f);

    std::vector<float> shiftedFilter { 1.0f };

    std::vector<std::complex<float>> roots;
   
    for (const auto& formant : formants) {
        if (formant.frequency < freqMin || formant.frequency > freqMax)
            continue;

        // We shift the pole frequency in the Mel frequency space.
        
        float melScaled = factor * 2595.0f * log10f(1.0f + formant.frequency / 700.0f);
        float freqScaled = 700.0f * (powf(10.0f, melScaled / 2595.0f) - 1.0f);

        if (freqScaled < freqMin || freqScaled > freqMax)
            continue;

        // Re-calculate bandwidth to have the same Q factor.
        float bandwidth = freqScaled / formant.frequency * formant.bandwidth;
        float frequency = freqScaled;

        // Re-calculate r and phi for the new pole.
        float r = expf(-M_PI * bandwidth / Fs);
        float phi = 2.0f * M_PI * frequency / Fs;

        roots.push_back(std::polar(r, phi));
    }

    const int npoles = roots.size();
    for (int i = 0; i < npoles; ++i) {
        roots.push_back(std::conj(roots[i]));
    }

    auto poly = createPolynomialFromRoots(roots);

    // Evaluate polynomial at f = 0Hz
    std::complex<float> y, dy;
    Analysis::evaluatePolynomialWithDerivative(poly, std::polar<float>(0.98f, 2.0f * M_PI * 10.0f / Fs), &y, &dy);

    float zeroGain = std::abs(1.0f / y);

    return {std::move(poly), zeroGain};
}
