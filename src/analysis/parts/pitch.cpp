//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "../../lib/Pitch/Pitch.h"

using namespace Eigen;

void Analyser::analysePitch()
{
    Pitch::Estimation est{};

    Pitch::estimate_MPM(x, fs, est);

    if (!est.isVoiced)
    {
        //Pitch::estimate_AMDF(x, fs, est, 60, 1000, 2.0, 0.001);
    }

    lastPitchFrame = est.isVoiced ? est.pitch : 0.0;
}
