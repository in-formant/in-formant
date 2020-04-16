#include <cmath>
#include "SineWave.h"

SineWave::SineWave()
    : gainDecay(0.99975), frequencyDecay(0.9),
      targetGain(0), targetFrequency(-1)
{
}

void SineWave::initWaveform(int sampleRate, int numChannels)
{
    mSampleRate = sampleRate;
    mTimeInSamples = 0;
    mFrequency = -1;
    mGain = 0;
    mChannels = numChannels;
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

    for (int i = 0; i < frameCount; ++i) {
   
        if (mFrequency < 0) {
            value = 0;
        }
        else {
            value = mGain * std::sin((2.0 * M_PI * mFrequency * mTimeInSamples) / (double) mSampleRate);
        }

        for (int ch = 0; ch < mChannels; ++ch) {
            output[mChannels * i + ch] += std::isnormal(value) ? value : 0;
        }

        mTimeInSamples += 1;

        if (mFrequency < 0 || (mFrequency > 0 && mTimeInSamples > mPeriodInSamples)) {
            if (mFrequency > 0) {
                mTimeInSamples -= mPeriodInSamples;
            }

            if (std::abs(mFrequency - targetFrequency) > 1e-6) {
                mFrequency = frequencyDecay * mFrequency + (1 - frequencyDecay) * targetFrequency;
                mPeriodInSamples = (double) mSampleRate / mFrequency;
            }
        }

        if (std::abs(mGain - targetGain) > 1e-9) {
            mGain = gainDecay * mGain + (1 - gainDecay) * targetGain;
        }
    }
}
