#include "../analysis/analysis.h"
#include "synthesis.h"
#include <iostream>

static rpm::vector<double> conv(const rpm::vector<double>& x, const rpm::vector<double>& y) {
    int lx = x.size();
    int ly = y.size();
    int lw = lx + ly - 1;
    rpm::vector<double> w(lw, 0.0);
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

rpm::vector<std::array<double, 6>> Synthesis::frequencyShiftFilter(const rpm::vector<Analysis::FormantData>& formants, double Fs, double factor)
{
    const double freqMin = 50.0;
    const double freqMax = Fs / 2.0 - 50.0;
    
    const double logMin = log2(freqMin);

    rpm::vector<std::complex<double>> roots;

    bool isFirstFormant = true;
    double scaledFirstFormant;

    for (const auto& formant : formants) {
        if (formant.frequency < freqMin || formant.frequency > freqMax)
            continue;

        // We shift the pole frequency in logarithmic frequency space.
       
        double logFrequency = log2(formant.frequency);
        double logScaled = logMin + factor * (logFrequency - logMin);
        double freqScaled = exp2(logScaled);

        if (freqScaled < freqMin || freqScaled > freqMax)
            continue;

        if (isFirstFormant) {
            scaledFirstFormant = freqScaled;
            isFirstFormant = false;
        }

        // Re-calculate bandwidth to have the same Q factor.
        double bandwidth = freqScaled / formant.frequency * formant.bandwidth;
        double frequency = freqScaled;

        // Re-calculate r and phi for the new pole.
        double r = exp(-M_PI * bandwidth / Fs);
        double phi = 2.0 * M_PI * frequency / Fs;

        roots.push_back(std::polar(r, phi));
    }

    const int npoles = roots.size();
    for (int i = 0; i < npoles; ++i) {
        roots.push_back(std::conj(roots[i]));
    }

    rpm::vector<std::complex<double>> zeros {
        std::polar(0.9, freqMin * 2.0 * M_PI / Fs),
    };

    const int nzeros = zeros.size();
    for (int i = 0; i < nzeros; ++i) {
        zeros.push_back(std::conj(zeros[i]));
    }

    return Analysis::zpk2sos(zeros, roots, 1.0);
}
