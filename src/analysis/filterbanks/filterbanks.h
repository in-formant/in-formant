#ifndef ANALYSIS_FILTERBANKS_H
#define ANALYSIS_FILTERBANKS_H

#include <Eigen/Sparse>
#include <cmath>
#include <functional>

namespace Analysis {

    Eigen::SparseMatrix<double> logFilterbank(double minFreqHz, double maxFreqHz, int melBinCount, int linearBinCount, double sampleRateHz);

    Eigen::SparseMatrix<double> melFilterbank(double minFreqHz, double maxFreqHz, int melBinCount, int linearBinCount, double sampleRateHz);

    Eigen::SparseMatrix<double> erbFilterbank(double minFreqHz, double maxFreqHz, int erbBinCount, int linearBinCount, double sampleRateHz);

}

inline double hz2mel(double f) {
    return 2595.0 * log10(1.0 + f / 700.0);
}

inline double mel2hz(double m) {
    return 700.0 * (pow(10.0, m / 2595.0) - 1.0);
}

inline double hz2erb(double f) {
    constexpr double A = 21.33228113095401739888262;
    return A * log10(1 + 0.00437 * f);
}

inline double erb2hz(double erb) {
    constexpr double A = 21.33228113095401739888262;
    return (pow(10.0, erb / A) - 1) / 0.00437;
}

inline double hz2log(double f) {
    return log2(f);
}

inline double log2hz(double l) {
    return exp2(l);
}

inline double fft2hz(int nbin, double fs, int nfft) {
    return (nbin * fs) / (2.0 * nfft);
}

inline int hz2fft(double f, double fs, int nfft) {
    return std::min((int) std::round((f * 2.0 * nfft) / fs), nfft - 1);
}

#endif // ANALYSIS_FILTERBANKS_H
