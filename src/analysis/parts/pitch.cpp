//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Pitch/Pitch.h"

using namespace Eigen;

void Analyser::analysePitch()
{
    Pitch::Estimation est{};

    Pitch::estimate_AMDF(x, fs, est, 60, 1000, 1.0, 0.1);
    
    if (est.isVoiced && est.pitch < 1000) {
        Pitch::estimate_MPM(x, fs, est);
    }

    lastPitchFrame = est.isVoiced ? est.pitch : 0;
}
