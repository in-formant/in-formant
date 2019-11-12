//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

using namespace Eigen;

void YAAPT::nlfer(
        const ArrayXd & data, double fs, const Params & prm,
        ArrayXd & energy, ArrayXb & vUvEnergy)
{
    int nfft = prm.fftLength;
    int frameSize = std::floor(prm.frameLength * fs / 1000.0);
    int frameJump = std::floor(prm.frameSpace * fs / 1000.0);

    // If normalized low-frequency is below this, assume unvoiced frame.
    double nlfer_thresh1 = prm.nlferThresh1;

    // Low frequency range for NLFER.
    int N_F0_min = std::round((prm.F0min * 2 / fs) * nfft);
    int N_F0_max = std::round((prm.F0max / fs) * nfft);

    // Spectrogram of the data
    ArrayXXcd specData;
    specgram(data, nfft, frameSize, frameSize - frameJump, specData);

    // Compute normalized low-frequency energy ratio
    ArrayXd frmEnergy = specData(seq(N_F0_min, N_F0_max), all).abs().colwise().sum();
    double avgEnergy = frmEnergy.mean();

    energy = frmEnergy / avgEnergy;
    vUvEnergy = (energy > nlfer_thresh1);
}