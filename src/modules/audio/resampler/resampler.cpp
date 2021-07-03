#include "resampler.h"
#include <cmath>
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
      mSrc(nullptr),
      mInRate(inRate),
      mOutRate(0)
{
    setupResampler();
}

Resampler::Resampler(int inRate, int outRate)
    : mId(sId++),
      mSrc(nullptr),
      mInRate(inRate),
      mOutRate(outRate)
{
    setupResampler();
}

Resampler::~Resampler()
{
}

void Resampler::setInputRate(int newInRate)
{
    if (mInRate != newInRate) {
        mInRate = newInRate;
    }
}

void Resampler::setOutputRate(int newOutRate)
{
    if (mOutRate != newOutRate) {
        mOutRate = newOutRate;
    }
}

void Resampler::setRate(int newInRate, int newOutRate)
{
    if (mInRate != newInRate || mOutRate != newOutRate) {
        mInRate = newInRate;
        mOutRate = newOutRate;
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

    return (int) ((double) inLength * mOutRate / mInRate + 0.5);
}

rpm::vector<double> Resampler::process(const rpm::vector<double>& inDouble)
{
    rpm::vector<float> in(inDouble.begin(), inDouble.end());
    rpm::vector<float> out(getExpectedOutLength(in.size()));

    SRC_DATA data;
    data.data_in = in.data();
    data.data_out = out.data();
    data.input_frames = in.size();
    data.output_frames = out.size();
    data.src_ratio = (double) mOutRate / (double) mInRate;
    data.end_of_input = 0;

    int error = src_process(mSrc, &data);

    if (error != 0) {
        throw std::runtime_error("Audio::Resampler#" + std::to_string(mId) + "] " + src_strerror(error));
    }

    out.resize(data.output_frames_gen);

    return rpm::vector<double>(out.begin(), out.end());
}

void Resampler::setupResampler()
{
    int error;

    mSrc = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);

    if (error != 0) {
        throw std::runtime_error("Audio::Resampler#" + std::to_string(mId) + "] " + src_strerror(error));
    }

    std::cout << "Audio::Resampler#" << mId << "] Created " << mInRate << " --> " << mOutRate << std::endl;
}
