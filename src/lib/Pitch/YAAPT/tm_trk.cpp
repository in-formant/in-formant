//
// Created by rika on 11/11/2019.
//

#include "YAAPT.h"

void YAAPT::tm_trk(
        const std::array<Eigen::ArrayXd, numFrames> & data, double fs,
        Eigen::ArrayXd & sPitch,
        double pStd, double pAvg, const Params & prm,
        Eigen::ArrayXXd & pitch, Eigen::ArrayXXd & merit)
{
    using namespace Eigen;

    int frameSize = data[0].size();
    int realNumFrames = numFrames;

    // This step is inserted for unifying numFrames of both temporal and spectral track
    int lenSpectral = sPitch.size();
    if (numFrames < lenSpectral) {
        sPitch = sPitch.head(numFrames);
    }
    if (numFrames > lenSpectral) {
        realNumFrames = lenSpectral;
    }

    const double meritBoost = prm.meritBoost;
    const int maxCands = prm.nccfMaxCands;
    const double freqThresh = 5 * pStd;

    // Determine the search range according spectral pitch track.
    ArrayXd sMin = sPitch - 2.0 * pStd;
    sMin = (sMin >= prm.F0min).select(sMin, prm.F0min);

    ArrayXd sMax = sPitch + 2.0 * pStd;
    sMax = (sMax <= prm.F0max).select(sMax, prm.F0max);

    // Initialisation
    pitch.setZero(maxCands, realNumFrames);
    merit.setZero(maxCands, realNumFrames);

    for (int n = 0; n < numFrames; ++n) {
        ArrayXd frame = data[n];

        // Compute pitch candidates and corresponding merit values,
        // up to maxCands per frame. Merit values are normalized (0-1)
        // and depend mainly on size of correlation peaks.
        int lagMin = std::floor(fs / sMax(n)) - 3;
        int lagMax = std::floor(fs / sMin(n)) + 3;

        // Compute correlation.
        ArrayXd phi;
        crs_corr(frame, lagMin, lagMax, phi);

        // The maxCands pitch candidates are collected into Pitch and Merit arrays.
        ArrayXd pitchCol, meritCol;
        cmp_rate(phi, fs, prm, maxCands, lagMin, lagMax, pitchCol, meritCol);

        pitch.col(n) = pitchCol;
        merit.col(n) = meritCol;
    }

    // The following lines increase merit for peaks with are very
    // close to in frequency to those peaks which are close to
    // smoothed F0 track from spectrogram, and decrease merit for peaks
    // which are not close to the smoothed F0 track obtained from spectrogram.

    for (int i = 0; i < maxCands; ++i) {
        ArrayXd diff = (pitch.row(i).transpose() - sPitch).abs();
        ArrayXb match1 = (diff < freqThresh);
        ArrayXd match = match1.select(1 - diff / freqThresh, 0);
        merit.row(i) *= (1 + meritBoost) * match;
    }
}