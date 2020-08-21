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
    : mSoxrIoSpec(soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I)),
      mSoxrQualitySpec(soxr_quality_spec(RESAMPLER_QUALITY, 0)),
      mSoxrRuntimeSpec(soxr_runtime_spec(1)),
      mInRate(inRate),
      mOutRate(outRate)
{
    createResampler();
}

Resampler::~Resampler()
{
    soxr_delete(mSoxr);
}

void Resampler::setInputRate(int newInRate)
{
    soxr_delete(mSoxr);
    mInRate = newInRate;
    createResampler();
}

void Resampler::setOutputRate(int newOutRate)
{
    soxr_delete(mSoxr);
    mOutRate = newOutRate;
    createResampler();
}

void Resampler::setRate(int newInRate, int newOutRate)
{
    soxr_delete(mSoxr);
    mInRate = newInRate;
    mOutRate = newOutRate;
    createResampler();
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

    float ioRatio = (float) mInRate / (float) mOutRate;

    return ceil((float) outLength * ioRatio);
}

int Resampler::getExpectedOutLength(int inLength)
{
    if (inLength == 0) {
        return 0;
    }
    
    float ioRatio = (float) mInRate / (float) mOutRate;

    return ceil((float) inLength / ioRatio);
}

int Resampler::getDelay() const
{
    return soxr_delay(mSoxr);
}

void Resampler::clear()
{
    soxr_clear(mSoxr);
}

void Resampler::process(const float *pIn, int inLength, float *pOut, int outLength)
{
    size_t inDone = 0;
    size_t outDone = 0;
    int inRemaining = inLength;
    int outRemaining = outLength;

    while (inRemaining > 0 && outRemaining > 0) {
        size_t localInDone;
        size_t localOutDone;

        err = soxr_process(mSoxr,
                        std::next(pIn, inDone), inRemaining, &localInDone,
                        std::next(pOut, outDone), outRemaining, &localOutDone);
        checkError();
        
        inDone += localInDone;
        outDone += localOutDone;
        inRemaining -= localInDone;
        outRemaining -= localOutDone;
    }
}

void Resampler::createResampler()
{
    mSoxr = soxr_create(mInRate, mOutRate, 1, &err, &mSoxrIoSpec, &mSoxrQualitySpec, &mSoxrRuntimeSpec);
    checkError();
    
    std::cout << "Audio::Resampler] Using engine " << soxr_engine(mSoxr) << std::endl;
}

void Resampler::checkError()
{
    if (err) {
        throw std::runtime_error(std::string("Audio::Resampler] ") + soxr_strerror(err));
    }
}
