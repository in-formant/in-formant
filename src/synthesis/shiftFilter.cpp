#include "../analysis/analysis.h"
#include "synthesis.h"
#include <iostream>

rpm::vector<std::array<double, 6>> Synthesis::frequencyShiftFilter(const rpm::vector<Analysis::FormantData>& formants, double Fs, double factor)
{
    const double freqMin = 50.0;
    const double freqMax = Fs / 2.0 - 50.0;
    
    const double logMin = log2(freqMin);

    rpm::vector<std::complex<double>> roots;

    for (const auto& formant : formants) {
        if (formant.frequency < freqMin || formant.frequency > freqMax)
            continue;

        // We shift the pole frequency in logarithmic frequency space.
       
        double logFrequency = log2(formant.frequency);
        double logScaled = logMin + factor * (logFrequency - logMin);
        double freqScaled = exp2(logScaled);

        if (freqScaled < freqMin || freqScaled > freqMax)
            continue;

        // Re-calculate bandwidth to have the same Q factor.
        double bandwidth = freqScaled / formant.frequency * formant.bandwidth;
        double frequency = freqScaled;

        // Re-calculate r and phi for the new pole.
        double r = exp(-M_PI * bandwidth / Fs);
        double phi = 2.0 * M_PI * frequency / Fs;

        roots.push_back(std::polar(r, phi));
    }

    const int npoles = (int) roots.size();
    for (int i = 0; i < npoles; ++i) {
        roots.push_back(std::conj(roots[i]));
    }

    rpm::vector<std::complex<double>> zeros {
        std::polar(0.9, freqMin * 2.0 * M_PI / Fs),
    };

    const int nzeros = (int) zeros.size();
    for (int i = 0; i < nzeros; ++i) {
        zeros.push_back(std::conj(zeros[i]));
    }

    return Analysis::zpk2sos(zeros, roots, 1.0);
}
