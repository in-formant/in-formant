#include "fft.h"

rpm::vector<double> Analysis::fft_n(Analysis::RealFFT& fft, const rpm::vector<double>& signal)
{
    const int nfft = fft.getInputLength();
    const int n = signal.size();

    if (n <= nfft) {
        for (int i = 0; i < nfft; ++i) {
            fft.input(i) = 0.0;
        }

        for (int j = 0; j < n; ++j) {
            double sample = signal[j];
            double window = 0.5 - 0.5 * cos((2.0 * M_PI * j) / (n - 1));

            int i = nfft / 2 - n / 2 + j;

            fft.input(i) = sample * window;
        }
    }
    else {
        const int N = nfft - 1;
        
        for (int j = 0; j < nfft; ++j) {
            int i = n / 2 - nfft / 2 + j;
            
            double sample = signal[i];
            double window = 0.5 - 0.5 * cos((2.0 * M_PI * j) / N);

            fft.input(j) = sample * window;
        }
    }

    fft.computeForward();

    rpm::vector<double> h(fft.getOutputLength());
    for (int k = 0; k < fft.getOutputLength(); ++k) {
        h[k] = std::abs(fft.output(k));
    }
    return h;
}
