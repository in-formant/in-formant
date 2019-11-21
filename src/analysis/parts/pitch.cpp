//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "../../lib/Pitch/Pitch.h"

using namespace Eigen;;

void Analyser::analysePitch()
{
    // Estimate pitch with AMDF. It will be refined periodically with YAAPT.
    Pitch::Estimation est{};

    Pitch::estimate_AMDF(x, fs, est, 60, 700, 2.0, 0.1);

    pitchTrack.pop_front();
    pitchTrack.push_back(est.isVoiced ? est.pitch : 0.0);
}