#include "../modules/math/constants.h"
#include "../analysis/analysis.h"
#include "synthesis.h"
#include <iostream>

static std::vector<double> conv(const std::vector<double>& x, const std::vector<double>& y) {
    int lx = x.size();
    int ly = y.size();
    int lw = lx + ly - 1;
    std::vector<double> w(lw, 0.0);
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

std::vector<std::array<double, 6>> Synthesis::frequencyShiftFilter(const std::vector<Analysis::FormantData>& formants, double Fs, double factor)
{
    const double freqMin = 50.0;
    const double freqMax = Fs / 2.0 - 50.0;
    
    const double melMin = 2595.0 * log10f(1.0 + freqMin / 700.0);
    const double melMax = 2595.0 * log10f(1.0 + freqMax / 700.0);

    std::vector<std::complex<double>> roots;
   
    for (const auto& formant : formants) {
        if (formant.frequency < freqMin || formant.frequency > freqMax)
            continue;

        // We shift the pole frequency in the Mel frequency space.
        
        double melScaled = factor * 2595.0 * log10f(1.0 + formant.frequency / 700.0);
        double freqScaled = 700.0 * (powf(10.0, melScaled / 2595.0) - 1.0);

        if (freqScaled < freqMin || freqScaled > freqMax)
            continue;

        // Re-calculate bandwidth to have the same Q factor.
        double bandwidth = freqScaled / formant.frequency * formant.bandwidth;
        double frequency = freqScaled;

        // Re-calculate r and phi for the new pole.
        double r = expf(-M_PI * bandwidth / Fs);
        double phi = 2.0 * M_PI * frequency / Fs;

        roots.push_back(std::polar(r, phi));
    }

    const int npoles = roots.size();
    for (int i = 0; i < npoles; ++i) {
        roots.push_back(std::conj(roots[i]));
    }

    return Analysis::zpk2sos({}, roots, 1.0);
}
