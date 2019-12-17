//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Signal/Resample.h"

using namespace Eigen;

void Analyser::resampleAudio(double newFs) {
    x = std::move(Resample::resample(x, fs, newFs, 50));
    fs = newFs;
}
