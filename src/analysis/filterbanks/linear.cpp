#include "filterbanks.h"

Eigen::SparseMatrix<double> Analysis::linearFilterbank(double minFreqHz, double maxFreqHz, int linearBinCount, int fftBinCount, double sampleRateHz)
{
    // Create linearBinCount linearly spaced values between these extreme freq values.
    Eigen::ArrayXd centerFrequenciesHz = Eigen::ArrayXd::LinSpaced(linearBinCount, minFreqHz, maxFreqHz);

    double freqsPerBin = (maxFreqHz - minFreqHz) / (double) (linearBinCount - 1);

    double hzStart = minFreqHz - freqsPerBin;
    int fftBinStart = hz2fft(hzStart, sampleRateHz, fftBinCount);

    double hzStop = maxFreqHz + freqsPerBin;
    int fftBinStop = hz2fft(hzStop, sampleRateHz, fftBinCount);

    // Map each center frequency to the closest fft bin index.
    Eigen::ArrayXi fftBinIndices = centerFrequenciesHz.unaryExpr(
            std::bind(hz2fft, std::placeholders::_1, sampleRateHz, fftBinCount));

    // Create filterbank matrix.
    std::vector<Eigen::Triplet<double>> tripletList;

    for (int linBin = 0; linBin < linearBinCount; ++linBin) {
        int centerFreqFftBin = fftBinIndices(linBin);

        // Create a triangular filter having the current center freq.
        // The filter will start with 0 response at left_bin (if it exists)
        // and ramp up to 1.0 at center_freq_linear_bin, and then ramp
        // back down to 0 response at right_bin (if it exists).
        
        // Create the left side of the triangular filter that ramps up
        // from 0 to a response of 1 at the center frequency.
        if (centerFreqFftBin > 1) {
            // It is possible to create the left triangular filter.
            int leftBin;

            if (linBin == 0) {
                // Since this is the first center frequency, the left side
                // must start ramping up from linear bin 0 or 1 mel bin before the center freq.
                leftBin = std::max(0, fftBinStart);
            }
            else {
                // Start ramping up from the previous center frequency bin.
                leftBin = fftBinIndices(linBin - 1);
            }
            for (int fBin = leftBin; fBin <= centerFreqFftBin; ++fBin) {
                if (centerFreqFftBin - leftBin > 0) {
                    double response = (double) (fBin - leftBin) / (double) (centerFreqFftBin - leftBin);
                    tripletList.emplace_back(linBin, fBin, response);
                }
            }
        }

        // Create the right side of the triangular filter that ramps down
        // from 1 to 0.
        if (centerFreqFftBin < fftBinCount - 2) {
            // It is possible to create the right triangular filter.
            int rightBin;

            if (linBin == linearBinCount - 1) {
                // Since this is the last mel bin, we must ramp down to response of 0
                // at the last linear freq bin.
                rightBin = std::min(fftBinCount - 1, fftBinStop);
            }
            else {
                rightBin = fftBinIndices(linBin + 1);
            }
            for (int fBin = centerFreqFftBin; fBin <= rightBin; ++fBin) {
                if (rightBin - centerFreqFftBin > 0) {
                    double response = (double) (rightBin - fBin) / (double) (rightBin - centerFreqFftBin);
                    tripletList.emplace_back(linBin, fBin, response);
                }
            }
        }

        tripletList.emplace_back(linBin, centerFreqFftBin, 1.0);
    }

    Eigen::SparseMatrix<double> filterbank(linearBinCount, fftBinCount);
    filterbank.setFromTriplets(tripletList.begin(), tripletList.end());

    for (int linBin = 0; linBin < linearBinCount; ++linBin) {
        int centerFreqFftBin = fftBinIndices(linBin);
        filterbank.coeffRef(linBin, centerFreqFftBin) = 1.0;
    }

    filterbank.makeCompressed();

    return filterbank;
}
