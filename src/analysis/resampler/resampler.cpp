#include "resampler.h"
#include "../fft/fft.h"
#include <cmath>

std::vector<float> Analysis::resample(const std::vector<float>& x, float srcFs, float dstFs)
{
    const int n = x.size();

    std::vector<double> z(n);

    float upFactor = dstFs / srcFs;
    
    if (upFactor < 1) {
        constexpr int antiTurnAround = 1000;
        constexpr int numberOfPaddingSides = 2;
        int nfft = 1;
        while (nfft < n + antiTurnAround * numberOfPaddingSides)
            nfft *= 2;

        ComplexFFT fft(nfft);
        for (int i = 0; i < nfft; ++i) {
            fft.data(i) = 0.0;
        }
        for (int i = 0; i < n; ++i) {
            fft.data(antiTurnAround + i) = x[i];
        }
        fft.computeForward();
        for (int i = std::floor(upFactor * nfft); i < nfft; ++i) {
            fft.data(i) = 0.0;
        }
        fft.computeBackward();

        for (int i = 0; i < n; ++i) {
            z[i] = std::abs(fft.data(antiTurnAround + i)) / (double) (2 * nfft);
        }
    }
    else {
        for (int i = 0; i < n; ++i) {
            z[i] = x[i];
        }
    }

    const int numberOfSamples = std::round(n * dstFs / srcFs);
    
    std::vector<float> y(numberOfSamples);

    for (int i = 0; i < numberOfSamples; ++i) {
        double index = (i * srcFs) / dstFs;
        int leftSample = std::floor(index);
        double frac = index - leftSample;
        y[i] = (leftSample < 0 || leftSample >= n - 1) ? 0.0f :
                (1 - frac) * z[leftSample] + frac * z[leftSample + 1];
    }

    return y;
}
