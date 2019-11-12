//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

using namespace Eigen;

void YAAPT::peaks(
        const ArrayXd & data, double delta, int maxPeaks, const Params & prm,
        ArrayXd & pitch, ArrayXd & merit)
{
    double peak_thresh1 = prm.shcThresh1;
    double peak_thresh2 = prm.shcThresh2;

    double epsilon = 1e-13;

    // Window width in samples
    int width = std::floor(prm.shcPWidth / delta);
    if (width % 2 == 0)
        width++;

    // The center of the window is defined as the peak location
    int center = std::ceil(width / 2.0);

    // Lowest frequency at which F0 is allowed
    int minLag = std::floor(prm.F0min / delta - width / 4);
    // Highest frequency at which F0 is allowed
    int maxLag = std::floor(prm.F0max / delta - width / 4);

    if (minLag < 0) {
        minLag = 0;
    }
    if (maxLag >= data.size() - width) {
        maxLag = data.size() - 1 - width;
    }

    pitch.setZero(maxPeaks);
    merit.setZero(maxPeaks);

    double avgData = data(seq(minLag, maxLag)).mean();

    // Step 1.
    // Find all peaks for search range.

    int numPeaks = 0;
    for (int n = minLag; n <= maxLag; ++n) {
        int lag;
        double y = data.segment(n, width).maxCoeff(&lag);
        // Find peaks which are larger than threshold
        if (lag == center && y > peak_thresh2 * avgData) {
            // Note pitch(0) = delta, pitch(1) = 2 * delta...
            // Convert FFT indices to Pitch in Hz
            pitch(numPeaks) = (n + center) * delta;
            merit(numPeaks) = y;
            numPeaks++;
        }
    }

    // Step 2.
    // Be sure there is a large peak.

    if (merit.maxCoeff() / avgData < peak_thresh1) {
        pitch.setZero(maxPeaks);
        merit.setOnes(maxPeaks);
        return;
    }

    // Step 3.
    // Order the peaks according to size, considering at most maxPeaks.
    std::vector<Index> idx(maxPeaks);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(),
              [&merit](Index i, Index j) { return merit(i) > merit(j); });
    merit = merit(idx);
    pitch = pitch(idx);
    numPeaks = std::min(numPeaks, maxPeaks);

    // Step 4.
    // Insert candidates to reduce pitch doubling and pitch halving, if needed
    if (numPeaks > 0) {
        // If best peak has F < this, insert peak at 2F
        if (pitch(0) > prm.F0double) {
            numPeaks = std::min(numPeaks + 1, maxPeaks);
            pitch(numPeaks - 1) = pitch(0) / 2.0;
            merit(numPeaks - 1) = prm.meritExtra * merit(0);
        }

        // If best peak has F > this, insert peak at 0.5F
        if (pitch(0) < prm.F0half) {
            numPeaks = std::min(numPeaks + 1, maxPeaks);
            pitch(numPeaks - 1) = 2.0 * pitch(0);
            merit(numPeaks - 1) = prm.meritExtra * merit(0);
        }

        // Fill in frames with less than maxPeaks with best choice.
        if (numPeaks < maxPeaks) {
            pitch(seq(numPeaks - 1, last)) = pitch(0);
            merit(seq(numPeaks - 1, last)) = merit(0);
        }
    }
    else {
        pitch.setZero(maxPeaks);
        merit.setOnes(maxPeaks);
    }
}