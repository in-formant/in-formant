//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "Signal/Resample.h"

using namespace Eigen;

static ArrayXd resample(ma_resampler * resampler, ArrayXd x) {
    ArrayXf xf = x.cast<float>();

    ma_uint64 inCount = x.size();
    ma_uint64 outCount = ma_resampler_get_expected_output_frame_count(resampler, inCount);

    float *yf = new float[outCount];

    ma_resampler_process_pcm_frames(resampler, xf.data(), &inCount, yf, &outCount);

    ArrayXd y = Map<ArrayXf>(yf, outCount).cast<double>();

    delete[] yf;

    return std::move(y);
}

void Analyser::resampleAudio() {
    x = resample(&resampler, x);
    fs = resampler.config.sampleRateOut;

    /*if (nsamples > fftSamples) {
        x_fft = x.segment(x.size() / 2 - nfft / 2, nfft);
    }
    else {
        x_fft = std::move(x);
        x = x_fft.head(int(frameSamples * newFs / fs));
    }*/
}
