//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Pitch/Pitch.h"

using namespace Eigen;

void Analyser::analysePitch()
{
    Pitch::Estimation est{};

    //Pitch::estimate_AMDF(x, fs, est, 90, 1000, 5.0, 0.1);

    //Pitch::estimate_MPM(x, fs, est);

    //Pitch::estimate_YIN(x, fs, est, 0.15);

    Pitch::estimate_DynWav(x, fs, est, 9, 3000, 12, 0.4, lastPitchFrame);

    lastPitchFrame = est.isVoiced ? est.pitch : 0;
}
