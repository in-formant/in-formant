#include "resampler.h"
#include <string>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <algorithm>

using namespace Module::Audio;

std::atomic_int Resampler::sId(0);

Resampler::Resampler(int inRate)
    : mId(sId++),
      mSoxrIoSpec(soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I)),
      mSoxrQualitySpec(soxr_quality_spec(SOXR_32_BITQ, 0)),
      mSoxrRuntimeSpec(soxr_runtime_spec(1)),
      mSoxr(nullptr),
      mInRate(inRate),
      mOutRate(0)
{
}

Resampler::Resampler(int inRate, int outRate)
    : mId(sId++),
      mSoxrIoSpec(soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I)),
      mSoxrQualitySpec(soxr_quality_spec(SOXR_32_BITQ, 0)),
      mSoxrRuntimeSpec(soxr_runtime_spec(1)),
      mSoxr(nullptr),
      mInRate(inRate),
      mOutRate(outRate)
{
    createResampler();
}

Resampler::~Resampler()
{
    if (mSoxr != nullptr) {
        soxr_delete(mSoxr);
    }
}

void Resampler::setInputRate(int newInRate)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mInRate != newInRate) {
        mInRate = newInRate;
        createResampler();
    }
}

void Resampler::setOutputRate(int newOutRate)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mOutRate != newOutRate) {
        mOutRate = newOutRate;
        createResampler();
    }
}

void Resampler::setRate(int newInRate, int newOutRate)
{
    std::lock_guard<std::mutex> lock(mMutex); 
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

int Resampler::getRequiredInLength(int outLength) const
{
    if (outLength == 0) {
        return 0;
    }

    return (int) ((outLength + 1) * mInRate / mOutRate);
}

int Resampler::getExpectedOutLength(int inLength) const
{
    if (inLength == 0) {
        return 0;
    }

    return (int) (inLength * mOutRate / mInRate + 0.5);
}

double Resampler::getDelay() const
{
    return soxr_delay(mSoxr);
}

rpm::vector<double> Resampler::process(const double *pIn, int inLength)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (inLength == 0) {
        return {0.0f};
    }
    
    rpm::vector<float> in(pIn, pIn + inLength);

    int outLength = (int) (inLength * mOutRate / mInRate + 0.5);
    rpm::vector<float> out(outLength);

    soxr_process(
            mSoxr,
            in.data(),
            inLength,
            nullptr,
            out.data(),
            outLength,
            nullptr);
    
    return rpm::vector<double>(out.begin(), out.end());
}

void Resampler::createResampler()
{
    if (mSoxr != nullptr) {
        soxr_delete(mSoxr);
    }
    soxr_error_t err;
    mSoxr = soxr_create(mInRate, mOutRate, 1,
            &err,
            &mSoxrIoSpec,
            &mSoxrQualitySpec,
            &mSoxrRuntimeSpec);
    if (mSoxr == nullptr) {
        throw std::runtime_error(std::string("Audio::Resampler#") + std::to_string(mId) + "] Unable to create resampler: " + soxr_strerror(err));
    }
    std::cout << "Audio::Resampler#" << mId << "] Created " << mInRate << " --> " << mOutRate << std::endl;
}

rpm::vector<double> Resampler::oneShot(const double *pIn, int inLength, int src, int dst)
{
    size_t olen = (size_t) (inLength * dst / src + 0.5);
    rpm::vector<float> out(olen);
    size_t odone;
    soxr_oneshot(src, dst, 1,
                 pIn, inLength, nullptr,
                 out.data(), olen, &odone,
                 nullptr, nullptr, nullptr);
    out.resize(odone);
    return rpm::vector<double>(out.begin(), out.end());
}
