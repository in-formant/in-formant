//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Signal/Resample.h"

using namespace Eigen;

void Analyser::resampleAudio(double newFs) {
    x = Resample::resample(x, fs, newFs, 50);
    x_fft = Resample::resample(x_fft, fs, newFs, 50);
    fs = newFs;
}
