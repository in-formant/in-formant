#include "../analysis/analysis.h"
#include "synthesis.h"

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

std::vector<float> Synthesis::frequencyShiftFilter(const std::vector<float>& a, float Fs, float factor)
{
    std::vector<std::complex<float>> roots = Analysis::findRoots(a);

    const float freqMin = 50.0f;
    const float freqMax = Fs / 2.0f - 50.0f;
    
    const float melMin = 2595.0f * log10f(1.0f + freqMin / 700.0f);
    const float melMax = 2595.0f * log10f(1.0f + freqMax / 700.0f);

    std::vector<float> shiftedFilter { 1.0f };

    for (const auto& z : roots) {
        if (z.imag() < 0)
            continue;

        float r = std::abs(z);
        float phi = std::arg(z);
        
        Analysis::FormantData formant = Analysis::calculateFormant(r, phi, Fs);
        
        if (formant.frequency < freqMin || formant.frequency > freqMax)
            continue;

        // We shift the pole frequency in the Mel frequency space.
        
        float melScaled = factor * 2595.0f * log10f(1.0f + formant.frequency / 700.0f);
        float freqScaled = 700.0f * (powf(10.0f, melScaled / 2595.0f) - 1.0f);

        if (freqScaled < freqMin || freqScaled > freqMax)
            continue;

        // Re-calculate bandwidth to have the same Q factor.
        formant.bandwidth = freqScaled / formant.frequency * formant.bandwidth;
        formant.frequency = freqScaled;

        // Re-calculate r and phi for the new pole.
        r = expf(-M_PI * formant.bandwidth / Fs);
        phi = 2.0f * M_PI * formant.frequency / Fs;

        // Calculate the resonant filter for this formant.
        std::vector<float> formantFilter {
            1.0f,
            -2.0f * r * cosf(phi),
            r * r,
        };

        shiftedFilter = conv(shiftedFilter, formantFilter);
    }

    return shiftedFilter;
}
