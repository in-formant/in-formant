#include <iostream>
#include "NoiseFilter.h"
#include "../Exceptions.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"
#include "FFT/FFT.h"
#include "../log/simpleQtLogger.h"

using namespace Eigen;

NoiseFilter::NoiseFilter()
    : gainDecay(0.98), filterDecay(0.95),
      targetGain(0), filterPassCount(4)
{
    noiseIn.reserve(MAX_BUFFER_SIZE);
    noiseOut.reserve(MAX_BUFFER_SIZE);
    noiseRes.reserve(MAX_BUFFER_SIZE);

    memoryOut.resize(filterPassCount, std::deque<float>(30, 0.0));
}

NoiseFilter::~NoiseFilter()
{
    ma_resampler_uninit(&resampler);
}

void NoiseFilter::initOutput(int sampleRate, int numChannels)
{
    ma_noise_config noiseConfig = ma_noise_config_init(ma_format_f32, numChannels, ma_noise_type_white, time(nullptr), 1.0);
    noiseConfig.duplicateChannels = MA_TRUE;

    if (ma_noise_init(&noiseConfig, &noise) != MA_SUCCESS) {
        LS_FATAL("Unable to initialise noise generator");
        throw AudioException("Unable to initialise noise generator");
    }

    ma_resampler_config resConfig = ma_resampler_config_init(ma_format_f32, numChannels, 7000, sampleRate, ma_resample_algorithm_linear);
    resConfig.linear.lpfOrder = 2;

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
    targetGain = playing ? 0.015 : 0;
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

    ma_uint64 inCount = ma_resampler_get_required_input_frame_count(&resampler, frameCount);

    ma_uint64 outLength = mChannels * frameCount;
    ma_uint64 inLength = mChannels * inCount;

    this->noiseIn.resize(inLength);
    this->noiseOut.resize(inLength);
    this->noiseRes.resize(outLength);

    // Generate noise
    ma_noise_read_pcm_frames(&noise, this->noiseIn.data(), inLength);
    
    // Apply filter 
    applyFilter();
    
    // Resample
    ma_resampler_process_pcm_frames(&resampler, this->noiseOut.data(), &inLength, this->noiseRes.data(), &outLength);

    // Apply gain
    for (int i = 0; i < signed(outLength); ++i) {
        output[i] += mGain * this->noiseRes[i];
    }
    
    if (std::abs(mGain - targetGain) > 1e-9) {
        mGain = gainDecay * mGain + (1 - gainDecay) * targetGain;
    }

    filter = filterDecay * filter + (1 - filterDecay) * targetFilter;
}

void NoiseFilter::applyFilter()
{
    const int count = noiseIn.size() / mChannels;
    const int nfilt = filter.size();

    std::vector<double> in(count);
    std::vector<double> out(count);
    for (int i = 0; i < count; ++i) {
        in[i] = 0;
        for (int ch = 0; ch < mChannels; ++ch) {
            in[i] += noiseIn[mChannels * i + ch];
        }
    }

    // All but one passes are LPC filtering
    for (int pass = 0; pass < filterPassCount; ++pass) {
        
        auto& memory = this->memoryOut[pass];

        for (int i = 0; i < count; ++i) {

            double val = in[i];

            for (int j = 1; j < nfilt; ++j) {
                val -= (filter[j] * memory[j-1]) / filter[0];
            }

            out[i] = val;

            memory.pop_back();
            memory.push_front(val);
        }

        for (int i = 0; i < count; ++i) {
            in[i] = out[i];
        }
    }

    double maxAmp = std::numeric_limits<double>::min();

    for (int i = 0; i < count; ++i) {
        if (abs(out[i]) > maxAmp) {
            maxAmp = abs(out[i]);
        }
    }

    for (int i = 0; i < count; ++i) {
        for (int ch = 0; ch < mChannels; ++ch) {
            noiseOut[mChannels * i + ch] = out[i] / maxAmp / (double) mChannels;
        }
    }

}

