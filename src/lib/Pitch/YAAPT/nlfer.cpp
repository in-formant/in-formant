//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

void YAAPT::nlfer(
        const AudioFrames & data, double fs, const Params & prm,
        RefXd energy, RefXb vUvEnergy)
{
    using namespace Eigen;

    int nfft = prm.fftLength;
    int frameSize = data[0].size();

    // If normalized low-frequency is below this, assume unvoiced frame.
    double nlfer_thresh1 = prm.nlferThresh1;

    // Low frequency range for NLFER.
    int N_F0_min = round((prm.F0min * 2 / fs) * nfft);
    int N_F0_max = round((prm.F0max / fs) * nfft);

    // Spectrogram of the data
    ArrayXXcd specData(nfft, numFrames);
    specgram(data, nfft, specData);

    // Compute normalized low-frequency energy ratio
    ArrayXd frmEnergy = specData(seq(N_F0_min, N_F0_max), all).abs().colwise().sum();
    double avgEnergy = frmEnergy.mean();

    energy = frmEnergy / avgEnergy;
    vUvEnergy = (energy > nlfer_thresh1);
}
