//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include <pitch_detection.h>
#include "../../lib/Pitch/Pitch.h"

using namespace Eigen;

void Analyser::analysePitch()
{
    Pitch::Estimation est{};

    Pitch::estimate_AMDF(x, fs, est, 80, 700, 5.0, 0.1);

    double pitch = est.pitch;

    if (est.isVoiced) {
        const int numSamples = CAPTURE_SAMPLE_COUNT(fs);

        std::vector<float> xvec(numSamples);
        Map<ArrayXf>(xvec.data(), numSamples) = x.cast<float>();

        //pitch = mpm->pitch(xvec, fs);
    }

    pitchTrack.pop_front();
    pitchTrack.push_back(est.isVoiced ? pitch : 0.0);
}
