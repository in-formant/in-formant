#include "fft.h"

static rpm::map<int, rpm::vector<double>> windows;

static const rpm::vector<double>& getWindow(int N) {
    auto wit = windows.find(N);
    if (wit == windows.end()) {
        constexpr double a0 = 0.35875;
        constexpr double a1 = 0.48829;
        constexpr double a2 = 0.14128;
        constexpr double a3 = 0.01168;
        rpm::vector<double> w(N);
        for (int j = 0; j < N; ++j) { 
            w[j] = a0 - a1 * cos((2.0 * M_PI * j) / (N - 1))
                      + a2 * cos((4.0 * M_PI * j) / (N - 1));
                      - a3 * cos((6.0 * M_PI * j) / (N - 1));
        }
        windows[N] = w;
        return windows[N];
    }
    return wit->second;
}

rpm::vector<double> Analysis::fft_n(Analysis::RealFFT& fft, const rpm::vector<double>& signal)
{
    const int nfft = fft.getInputLength();
    const int n = signal.size();

    if (n <= nfft) {
        for (int i = 0; i < nfft; ++i) {
            fft.input(i) = 0.0;
        }

        auto w = getWindow(n);

        for (int j = 0; j < n; ++j) {
            double sample = signal[j];

            int i = nfft / 2 - n / 2 + j;

            fft.input(i) = sample * w[j];
        }
    }
    else {
        const int N = nfft - 1;
        
        auto w = getWindow(nfft);
        
        for (int j = 0; j < nfft; ++j) {
            int i = n / 2 - nfft / 2 + j;
            
            double sample = signal[i];

            fft.input(j) = sample * w[j];
        }
    }

    fft.computeForward();

    rpm::vector<double> h(fft.getOutputLength());
    for (int k = 0; k < fft.getOutputLength(); ++k) {
        h[k] = (fft.output(k) * std::conj(fft.output(k))).real();
    }
    return h;
}
