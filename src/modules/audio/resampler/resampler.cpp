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
      mSoxrQualitySpec(soxr_quality_spec(SOXR_VHQ, 0)),
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

    static std::mutex garbageMut;
    static std::vector<float> garbageIn, garbageOut;

    static soxr_io_spec_t ioSpec = soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
    static soxr_quality_spec_t qSpec = soxr_quality_spec(SOXR_QQ, 0);
    static soxr_runtime_spec_t rSpec = soxr_runtime_spec(1);
    
    int firstEstimate = (outLength * mInRate) / mOutRate + 100;
    size_t inLength;

    garbageMut.lock();

    garbageIn.resize(firstEstimate, 0.0f);
    garbageOut.resize(outLength);
    
    soxr_oneshot(
            mInRate, mOutRate, 1,
            garbageIn.data(), firstEstimate, &inLength,
            garbageOut.data(), outLength, nullptr,
            &ioSpec, &qSpec, &rSpec);

    garbageMut.unlock();

    return inLength;
}

int Resampler::getExpectedOutLength(int inLength)
{
    if (inLength == 0) {
        return 0;
    }

    static std::mutex garbageMut;
    static std::vector<float> garbageIn, garbageOut;

    static soxr_io_spec_t ioSpec = soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
    static soxr_quality_spec_t qSpec = soxr_quality_spec(SOXR_QQ, 0);
    static soxr_runtime_spec_t rSpec = soxr_runtime_spec(1);
    
    int firstEstimate = (inLength * mOutRate) / mInRate + soxr_delay(mSoxr) + 100;
    size_t outLength;

    garbageMut.lock();

    garbageIn.resize(inLength, 0.0f);
    garbageOut.resize(firstEstimate);
    
    soxr_oneshot(
            mInRate, mOutRate, 1,
            garbageIn.data(), inLength, nullptr,
            garbageOut.data(), firstEstimate, &outLength,
            &ioSpec, &qSpec, &rSpec);

    garbageMut.unlock();

    return outLength;
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

    /*while (outRemaining > 0) {
        size_t localOutDone;

        err = soxr_process(mSoxr,
                        pIn, 0, nullptr,
                        std::next(pOut, outDone), outRemaining, &localOutDone);
        checkError();
       
        outDone += localOutDone;
        outRemaining -= localOutDone;
    }*/
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
