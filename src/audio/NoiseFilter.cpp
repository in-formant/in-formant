#include <iostream>
#include "NoiseFilter.h"
#include "../Exceptions.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"
#include "FFT/FFT.h"
#include "../log/simpleQtLogger.h"

using namespace Eigen;

NoiseFilter::NoiseFilter()
    : gainDecay(0.999), filterDecay(0.6), 
      targetGain(0), filterPassCount(2)
{
    noiseIn.reserve(MAX_NOISE_BUFFER);
    noiseOut.reserve(MAX_NOISE_BUFFER);
    noiseRes.reserve(MAX_NOISE_BUFFER);

    memoryOut.resize(filterPassCount, rpm::deque<float>(30, 0.0));
}

NoiseFilter::~NoiseFilter()
{
    ma_resampler_uninit(&resampler);
}

void NoiseFilter::initOutput(int sampleRate, int numChannels)
{
    ma_noise_config noiseConfig = ma_noise_config_init(ma_format_f32, 1, ma_noise_type_white, time(nullptr), 1.0);

    if (ma_noise_init(&noiseConfig, &noise) != MA_SUCCESS) {
        LS_FATAL("Unable to initialise noise generator");
        throw AudioException("Unable to initialise noise generator");
    }

    ma_resampler_config resConfig = ma_resampler_config_init(ma_format_f32, 1, 7000, sampleRate, ma_resample_algorithm_linear);
    resConfig.linear.lpfOrder = 3;
    resConfig.linear.lpfNyquistFactor = 0.8;

    if (ma_resampler_init(&resConfig, &resampler)) {
        LS_FATAL("Unable to initialise noise resampler");
        throw AudioException("Unable to initialise noise resampler");
    }

    mSampleRateIn = 7000;
    mSampleRateOut = sampleRate;
    mChannels = numChannels;
    mGain = 0;
    targetFilter.setOnes(1);
    filter.setOnes(1);
}

void NoiseFilter::setPlaying(bool playing)
{
    targetGain = playing ? 0.05 : 0;
}

void NoiseFilter::setFilter(int sampleRateIn, const ArrayXd & a)
{ 
    std::lock_guard<std::mutex> guard(mutex);

    if (mSampleRateIn != sampleRateIn) {
        if (ma_resampler_set_rate(&resampler, sampleRateIn, mSampleRateOut) != MA_SUCCESS) {
            LS_FATAL("Unable to change noise resampler input rate");
            throw AudioException("Unable to change noise resampler input rate");
        }
        mSampleRateIn = sampleRateIn;
    }

    const int nfilt = filter.size();
    const int ntarget = a.size() + 1;
    if (ntarget > nfilt) {
        filter.conservativeResize(ntarget);
        filter.tail(ntarget - nfilt).setZero();
        targetFilter.resize(ntarget);
        targetFilter(0) = 1.0;
        targetFilter.tail(a.size()) = a;
    }
    else if (nfilt > ntarget) {
        targetFilter.setZero(nfilt);
        targetFilter(0) = 1.0;
        targetFilter.segment(1, a.size()) = a;
    }
    else {
        targetFilter.tail(a.size()) = a;
    }
}

void NoiseFilter::readFrames(float *output, int frameCount)
{
    std::lock_guard<std::mutex> guard(mutex);

    ma_uint64 outCount = frameCount;

    noiseRes.resize(outCount, 0.0);

    if (mGain > 1e-9) {
        ma_uint64 inCount = ma_resampler_get_required_input_frame_count(&resampler, frameCount);

        noiseIn.resize(inCount);
        noiseOut.resize(inCount);

        // Generate noise
        ma_noise_read_pcm_frames(&noise, noiseIn.data(), inCount);
        
        // Apply filter 
        applyFilter(noiseIn, noiseOut);
        
        // Resample
        ma_resampler_process_pcm_frames(&resampler, noiseOut.data(), &inCount, noiseRes.data(), &outCount);
    }

    // Apply gain
    for (int i = 0; i < signed(outCount); ++i) {
        for (int ch = 0; ch < mChannels; ++ch) {
            output[mChannels * i + ch] += mGain * noiseRes[i];
        }
        
        if (std::abs(mGain - targetGain) > 1e-9) {
            mGain = gainDecay * mGain + (1 - gainDecay) * targetGain;
        }
    }

    filter = filterDecay * filter + (1 - filterDecay) * targetFilter;
}

void NoiseFilter::applyFilter(rpm::vector<float>& noiseIn, rpm::vector<float>& noiseOut)
{
    const int length = noiseIn.size();
    const int nfilt = filter.size();

    rpm::vector<double> in(length);
    rpm::vector<double> out(length);
    for (int i = 0; i < length; ++i) {
        in[i] = noiseIn[i];
    }

    // LPC filtering
    for (int pass = 0; pass < filterPassCount; ++pass) {
        
        auto& memory = memoryOut[pass];

        for (int i = 0; i < length; ++i) {

            double val = in[i];

            for (int j = 1; j < nfilt; ++j) {
                val -= (filter[j] * memory[j-1]) / filter[0];
            }

            out[i] = val;

            memory.pop_back();
            memory.push_front(val);
        }

        for (int i = 0; i < length; ++i) {
            in[i] = out[i];
        }
    }

    // Minimum amplitude to be normalized is 1.
    double maxAmp = 1;

    for (int i = 0; i < length; ++i) {
        if (abs(out[i]) > maxAmp) {
            maxAmp = abs(out[i]);
        }
    }

    for (int i = 0; i < length; ++i) {
        noiseOut[i] = out[i] / maxAmp;
    }

}

