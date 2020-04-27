//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Pitch/Pitch.h"

using namespace Eigen;

void Analyser::analysePitch()
{
    Pitch::Estimation est{};

    switch (pitchAlg) {
        /*case CREPE:
            Pitch::estimate_CREPE(x_fft.head(1024), fs, est);
            break;*/
        case Wavelet:
            Pitch::estimate_DynWav(x, fs, est, 6, 3000, 12, 0.35, lastPitchFrame);
            break;
        case McLeod:
            Pitch::estimate_MPM(x, fs, est);
            break;
        case YIN:
            Pitch::estimate_YIN(x, fs, est, 0.30);
            break;
        case AMDF:
            Pitch::estimate_AMDF(x, fs, est, 90, 1000, 4.0, 0.1);
            break;
        default:
            est.isVoiced = false;
    }

    lastPitchFrame = est.isVoiced ? est.pitch : 0;
}
