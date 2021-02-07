#include "resampler.h"
#include <string>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <algorithm>

using namespace Module::Audio;

Resampler::Resampler(int inRate, int outRate)
    : mSoxrIoSpec(soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I)),
      mSoxrQualitySpec(soxr_quality_spec(SOXR_HQ, SOXR_DOUBLE_PRECISION | SOXR_VR)),
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
        soxr_set_io_ratio(
                mSoxr, mInRate / mOutRate,
                5.0 / 1000.0 * mOutRate);
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

    return inLength * mOutRate / mInRate + 0.5;
}

rpm::vector<double> Resampler::process(const double *pIn, int inLength)
{
    if (inLength == 0) {
        return {0.0f};
    }
    
    rpm::vector<float> in(pIn, pIn + inLength);

    int outLength = inLength * mOutRate / mInRate + 0.5;
    rpm::vector<float> out(outLength);

    size_t idone = 0;
    size_t odone = 0;

    do {
        soxr_process(
                mSoxr,
                in.data(),
                inLength,
                nullptr,
                out.data(),
                outLength,
                &odone);
    } while (odone < outLength);
    
    return rpm::vector<double>(out.begin(), out.end());
}

void Resampler::createResampler()
{
    std::cout << "Audio::Resampler] Created " << mInRate << " --> " << mOutRate << std::endl;
    if (mSoxr != nullptr) {
        soxr_delete(mSoxr);
    }
    mSoxr = soxr_create(mInRate, mOutRate, 1,
            nullptr,
            &mSoxrIoSpec,
            &mSoxrQualitySpec,
            &mSoxrRuntimeSpec);
}

