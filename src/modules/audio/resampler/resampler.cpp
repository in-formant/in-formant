#include "resampler.h"
#include <string>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <vector>

using namespace Module::Audio;

Resampler::Resampler(int inRate, int outRate)
    : mInRate(inRate),
      mOutRate(outRate)
{
    createResampler();
}

void Resampler::setInputRate(int newInRate)
{
    if (mInRate != newInRate) {
        mInRate = newInRate;
        createResampler();
    }
}

void Resampler::setOutputRate(int newOutRate)
{
    if (mOutRate != newOutRate) {
        mOutRate = newOutRate;
        createResampler();
    }
}

void Resampler::setRate(int newInRate, int newOutRate)
{
    if (mInRate != newInRate || mOutRate != newOutRate) {
        mInRate = newInRate;
        mOutRate = newOutRate;
        createResampler();
    }
}

int Resampler::getInputRate() const
{
    return mInRate;
}

int Resampler::getOutputRate() const
{
    return mOutRate;
}

void Resampler::getRate(int *pInRate, int *pOutRate) const
{
    *pInRate = mInRate;
    *pOutRate = mOutRate;
}

int Resampler::getRequiredInLength(int outLength)
{
    if (outLength == 0) {
        return 0;
    }

    return (int) (((outLength - 0.5f) * mInRate) / mOutRate);
}

int Resampler::getExpectedOutLength(int inLength)
{
    if (inLength == 0) {
        return 0;
    }

    return (int) ((mOutRate * inLength) / mInRate + 0.5f);
}

int Resampler::getDelay() const
{
    return mResampler->getLatency() + mResampler->getLatencyFrac();
}

void Resampler::clear()
{
    mResampler->clear();
}

std::vector<double> Resampler::process(double *pIn, int inLength)
{
    if (inLength == 0) {
        return {0.0f};
    }

    std::vector<double> out((inLength * mOutRate) / mInRate);
    mResampler->oneshot(pIn, inLength,
                        out.data(), out.size());
    return out;
}

void Resampler::createResampler()
{
    std::cout << "Audio::Resampler] Created " << mInRate << " --> " << mOutRate << std::endl;
    mResampler.reset(new r8b::CDSPResampler(mInRate, mOutRate, 16384));
}

