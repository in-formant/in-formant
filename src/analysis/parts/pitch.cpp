//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "../../lib/Pitch/Pitch.h"

using namespace Eigen;;

void Analyser::analysePitch()
{
    // Estimate pitch with AMDF, then refine with AMDF.
    Pitch::Estimation est{};

    Pitch::estimate_AMDF(x, fs, est, 70, 600, 1.0, 0.01);
    if (est.isVoiced && (est.pitch > 70 && est.pitch < 600)) {

        const double pitch = est.pitch;
        Pitch::estimate_AMDF(x, fs, est, 0.8 * pitch, 1.2 * pitch, 1.5, 0.01);
        est.isVoiced &= (est.pitch > 0.8 * pitch && est.pitch < 1.2 * pitch);
    }
    else {
        est.isVoiced = false;
    }

    pitchTrack.pop_front();
    pitchTrack.push_back(est.isVoiced ? est.pitch : 0.0);
}