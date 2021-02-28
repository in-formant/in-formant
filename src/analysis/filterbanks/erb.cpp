#include "filterbanks.h"

Eigen::SparseMatrix<double> Analysis::erbFilterbank(double minFreqHz, double maxFreqHz, int erbBinCount, int linearBinCount, double sampleRateHz)
{
    double minErbs = hz2erb(minFreqHz);
    double maxErbs = hz2erb(maxFreqHz);

    // Create erbBinCount linearly spaced values between these extreme erb values.
    Eigen::ArrayXd erbLinSpaced = Eigen::ArrayXd::LinSpaced(erbBinCount, minErbs, maxErbs);

    // Map each of these erb values back into linear frequencies (Hz).
    Eigen::ArrayXd centerFrequenciesHz = erbLinSpaced.unaryExpr(std::ptr_fun(erb2hz));

    double erbsPerBin = (maxErbs - minErbs) / (double) (erbBinCount - 1);

    double erbsStart = minErbs - erbsPerBin;
    double hzStart = erb2hz(erbsStart);
    int fftBinStart = hz2fft(hzStart, sampleRateHz, linearBinCount);

    double erbsEnd = maxErbs + erbsPerBin;
    double hzStop = erb2hz(erbsEnd);
    int fftBinStop = hz2fft(hzStop, sampleRateHz, linearBinCount);

    // Map each center frequency to the closest fft bin index.
    Eigen::ArrayXi linearBinIndices = centerFrequenciesHz.unaryExpr(
            std::bind(hz2fft, std::placeholders::_1, sampleRateHz, linearBinCount));

    // Create filterbank matrix.
    std::vector<Eigen::Triplet<double>> tripletList;

    for (int erbBin = 0; erbBin < erbBinCount; ++erbBin) {
        int centerFreqLinearBin = linearBinIndices(erbBin);

        // Create a triangular filter having the current center freq.
        // The filter will start with 0 response at left_bin (if it exists)
        // and ramp up to 1.0 at center_freq_linear_bin, and then ramp
        // back down to 0 response at right_bin (if it exists).
        
        // Create the left side of the triangular filter that ramps up
        // from 0 to a response of 1 at the center frequency.
        if (centerFreqLinearBin > 1) {
            // It is possible to create the left triangular filter.
            int leftBin;

            if (erbBin == 0) {
                // Since this is the first center frequency, the left side
                // must start ramping up from linear bin 0 or 1 mel bin before the center freq.
                leftBin = std::max(0, fftBinStart);
            }
            else {
                // Start ramping up from the previous center frequency bin.
                leftBin = linearBinIndices[erbBin - 1];
            }
            for (int fBin = leftBin; fBin <= centerFreqLinearBin; ++fBin) {
                if (centerFreqLinearBin - leftBin > 0) {
                    double response = (double) (fBin - leftBin) / (double) (centerFreqLinearBin - leftBin);
                    tripletList.emplace_back(erbBin, fBin, response);
                }
            }
        }

        // Create the right side of the triangular filter that ramps down
        // from 1 to 0.
        if (centerFreqLinearBin < linearBinCount - 2) {
            // It is possible to create the right triangular filter.
            int rightBin;

            if (erbBin == erbBinCount - 1) {
                // Since this is the last mel bin, we must ramp down to response of 0
                // at the last linear freq bin.
                rightBin = std::min(linearBinCount - 1, fftBinStop);
            }
            else {
                rightBin = linearBinIndices[erbBin + 1];
            }
            for (int fBin = centerFreqLinearBin; fBin <= rightBin; ++fBin) {
                if (rightBin - centerFreqLinearBin > 0) {
                    double response = (double) (rightBin - fBin) / (double) (rightBin - centerFreqLinearBin);
                    tripletList.emplace_back(erbBin, fBin, response);
                }
            }
        }

        tripletList.emplace_back(erbBin, centerFreqLinearBin, 1.0);
    }

    Eigen::SparseMatrix<double> filterbank(erbBinCount, linearBinCount);
    filterbank.setFromTriplets(tripletList.begin(), tripletList.end());

    for (int erbBin = 0; erbBin < erbBinCount; ++erbBin) {
        int centerFreqLinearBin = linearBinIndices(erbBin);
        filterbank.coeffRef(erbBin, centerFreqLinearBin) = 1.0;
    }

    filterbank.makeCompressed();

    return filterbank;
}
