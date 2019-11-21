//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include <pitch_detection.h>

using namespace Eigen;

void Analyser::analysePitch()
{
    std::vector<double> xvec;
    xvec.resize(x.size());
    Map<ArrayXd>(xvec.data(), x.size()) = x;

    double pitch = pitch::swipe<double>(xvec, fs);

    pitchTrack.pop_front();
    pitchTrack.push_back(pitch > 0 ? pitch : 0.0);
}
