//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "../../lib/Signal/Resample.h"

using namespace Eigen;

void Analyser::resampleAudio() {
    const double newFs = 2 * maximumFrequency;
    x = std::move(Resample::resample(x, fs, newFs, 50));
    fs = newFs;
}