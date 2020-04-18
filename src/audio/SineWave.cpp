#include <cmath>
#include "SineWave.h"
#include "../Exceptions.h"
#include "../log/simpleQtLogger.h"

SineWave::SineWave()
    : gainDecay(0.9975), frequencyDecay(0.95),
      targetGain(0), targetFrequency(-1),
      mSampleRateIn(96000)
{
    sineIn.reserve(MAX_SINE_BUFFER);
    sineOut.reserve(MAX_SINE_BUFFER);
}

SineWave::~SineWave()
{
    ma_resampler_uninit(&resampler);
}

void SineWave::initWaveform(int sampleRate, int numChannels)
{
    mSampleRateOut = sampleRate;
    mTimeInSamples = 0;
    mFrequency = -1;
    mGain = 0;
    mChannels = numChannels;

    ma_resampler_config resConfig = ma_resampler_config_init(ma_format_f32, 1, mSampleRateIn, sampleRate,
#ifndef Q_OS_WASM
            ma_resample_algorithm_speex);
    resConfig.speex.quality = 3;
#else
            ma_resample_algorithm_linear);
#endif

    if (ma_resampler_init(&resConfig, &resampler)) {
        LS_FATAL("Unable to initialise sine wave resampler");
        throw AudioException("Unable to initialise sine wave resampler");
    }
}

void SineWave::setPlaying(bool playing)
{
    targetGain = playing ? 0.05 : 0;
}

void SineWave::setFrequency(double frequency)
{
    targetFrequency = frequency;
}

void SineWave::readFrames(float *output, int frameCount)
{
    double value;

    ma_uint64 inCount = ma_resampler_get_required_input_frame_count(&resampler, frameCount);
    ma_uint64 outCount = frameCount;

    sineIn.resize(inCount);
    sineOut.resize(outCount);
    
    for (int i = 0; i < signed(inCount); ++i) {    
        if (mFrequency < 0) {
            value = 0;
        }
        else {
            value = mGain * std::sin((2.0 * M_PI * mFrequency * mTimeInSamples) / (double) mSampleRateIn);
        }

        sineIn[i] = value;

        mTimeInSamples += 1;

        if (mFrequency < 0 || (mFrequency > 0 && mTimeInSamples > mPeriodInSamples)) {
            if (mFrequency > 0) {
                mTimeInSamples -= mPeriodInSamples;
            }

            if (std::abs(mFrequency - targetFrequency) > 1e-6) {
                mFrequency = frequencyDecay * mFrequency + (1 - frequencyDecay) * targetFrequency;
                mPeriodInSamples = (double) mSampleRateIn / mFrequency;
            }
        }

        if (std::abs(mGain - targetGain) > 1e-9) {
            mGain = gainDecay * mGain + (1 - gainDecay) * targetGain;
        }
    }
    
    ma_resampler_process_pcm_frames(&resampler, sineIn.data(), &inCount, sineOut.data(), &outCount);
    
    for (int i = 0; i < frameCount; ++i) {
        for (int ch = 0; ch < mChannels; ++ch) {
            output[mChannels * i + ch] += sineOut[i];
        }
    }
}
