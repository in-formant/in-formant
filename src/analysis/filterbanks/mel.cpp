#include "filterbanks.h"

Eigen::SparseMatrix<double> Analysis::melFilterbank(double minFreqHz, double maxFreqHz, int melBinCount, int linearBinCount, double sampleRateHz)
{
    double minMels = hz2mel(minFreqHz);
    double maxMels = hz2mel(maxFreqHz);

    // Create melBinCount linearly spaced values between these extreme mel values.
    Eigen::ArrayXd melLinSpaced = Eigen::ArrayXd::LinSpaced(melBinCount, minMels, maxMels);

    // Map each of these mel values back into linear frequencies (Hz).
    Eigen::ArrayXd centerFrequenciesHz = melLinSpaced.unaryExpr(std::ptr_fun(mel2hz));

    double melsPerBin = (maxMels - minMels) / (double) (melBinCount - 1);

    double melsStart = minMels - melsPerBin;
    double hzStart = mel2hz(melsStart);
    int fftBinStart = hz2fft(hzStart, sampleRateHz, linearBinCount);

    double melsEnd = maxMels + melsPerBin;
    double hzStop = mel2hz(melsEnd);
    int fftBinStop = hz2fft(hzStop, sampleRateHz, linearBinCount);

    // Map each center frequency to the closest fft bin index.
    Eigen::ArrayXi linearBinIndices = centerFrequenciesHz.unaryExpr(
            std::bind(hz2fft, std::placeholders::_1, sampleRateHz, linearBinCount));

    // Create filterbank matrix.
    std::vector<Eigen::Triplet<double>> tripletList;

    for (int melBin = 0; melBin < melBinCount; ++melBin) {
        int centerFreqLinearBin = linearBinIndices(melBin);

        // Create a triangular filter having the current center freq.
        // The filter will start with 0 response at left_bin (if it exists)
        // and ramp up to 1.0 at center_freq_linear_bin, and then ramp
        // back down to 0 response at right_bin (if it exists).
        
        // Create the left side of the triangular filter that ramps up
        // from 0 to a response of 1 at the center frequency.
        if (centerFreqLinearBin > 1) {
            // It is possible to create the left triangular filter.
            int leftBin;

            if (melBin == 0) {
                // Since this is the first center frequency, the left side
                // must start ramping up from linear bin 0 or 1 mel bin before the center freq.
                leftBin = std::max(0, fftBinStart);
            }
            else {
                // Start ramping up from the previous center frequency bin.
                leftBin = linearBinIndices[melBin - 1];
            }
            for (int fBin = leftBin; fBin <= centerFreqLinearBin; ++fBin) {
                if (centerFreqLinearBin - leftBin > 0) {
                    double response = (double) (fBin - leftBin) / (double) (centerFreqLinearBin - leftBin);
                    tripletList.emplace_back(melBin, fBin, response);
                }
            }
        }

        // Create the right side of the triangular filter that ramps down
        // from 1 to 0.
        if (centerFreqLinearBin < linearBinCount - 2) {
            // It is possible to create the right triangular filter.
            int rightBin;

            if (melBin == melBinCount - 1) {
                // Since this is the last mel bin, we must ramp down to response of 0
                // at the last linear freq bin.
                rightBin = std::min(linearBinCount - 1, fftBinStop);
            }
            else {
                rightBin = linearBinIndices[melBin + 1];
            }
            for (int fBin = centerFreqLinearBin; fBin <= rightBin; ++fBin) {
                if (rightBin - centerFreqLinearBin > 0) {
                    double response = (double) (rightBin - fBin) / (double) (rightBin - centerFreqLinearBin);
                    tripletList.emplace_back(melBin, fBin, response);
                }
            }
        }

        tripletList.emplace_back(melBin, centerFreqLinearBin, 1.0);
    }

    Eigen::SparseMatrix<double> filterbank(melBinCount, linearBinCount);
    filterbank.setFromTriplets(tripletList.begin(), tripletList.end());

    for (int melBin = 0; melBin < melBinCount; ++melBin) {
        int centerFreqLinearBin = linearBinIndices(melBin);
        filterbank.coeffRef(melBin, centerFreqLinearBin) = 1.0;
    }

    filterbank.makeCompressed();

    return filterbank;
}
