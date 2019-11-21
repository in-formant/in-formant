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
        //pitch_alloc::Yin<double> yin(CAPTURE_SAMPLE_COUNT(fs));

        /*const int numSamples = CAPTURE_SAMPLE_COUNT(fs);

        std::vector<float> xvec(numSamples);
        Map<ArrayXf>(xvec.data(), numSamples) = x.cast<float>();

        pitch = pitch::pmpm(xvec, fs);*/
    }

    pitchTrack.pop_front();
    pitchTrack.push_back(est.isVoiced ? pitch : 0.0);
}
