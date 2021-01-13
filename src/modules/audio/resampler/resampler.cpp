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
    : mResampler(nullptr),
      mInRate(inRate),
      mOutRate(outRate)
{
    createResampler();
}

Resampler::~Resampler()
{
    if (mResampler != nullptr) {
        speex_resampler_destroy(mResampler);
    }
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

    return (inLength * mOutRate + mInRate - 1) / mInRate;
}

int Resampler::getDelay() const
{
    return speex_resampler_get_output_latency(mResampler);
}

std::vector<double> Resampler::process(double *pIn, int inLength)
{
    if (inLength == 0) {
        return {0.0f};
    }

    speex_resampler_reset_mem(mResampler);
    speex_resampler_skip_zeros(mResampler);
   
    spx_uint32_t inLen = inLength;
    spx_uint32_t outLen = (inLen * mOutRate + mInRate - 1) / mInRate;
    
    std::vector<float> fin(pIn, pIn + inLen);
    std::vector<float> fout(outLen);

    speex_resampler_process_float(mResampler, 0, fin.data(), &inLen, fout.data(), &outLen);
    fout.resize(outLen);

    return std::vector<double>(fout.begin(), fout.end());
}

void Resampler::createResampler()
{
    std::cout << "Audio::Resampler] Created " << mInRate << " --> " << mOutRate << std::endl;
    if (mResampler != nullptr) {
        speex_resampler_destroy(mResampler);
    }
    mResampler = speex_resampler_init(chMono, mInRate, mOutRate, 7, nullptr);
}

