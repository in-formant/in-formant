//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Signal/Resample.h"

using namespace Eigen;

void Analyser::resampleAudio(double newFs) {
    x = Resample::resample(x, fs, newFs, 20);
    
    /*if (nsamples > fftSamples) {
        x_fft = x.segment(x.size() / 2 - nfft / 2, nfft);
    }
    else {
        x_fft = std::move(x);
        x = x_fft.head(int(frameSamples * newFs / fs));
    }*/

    fs = newFs;
}
