#include "resampler.h"
#include "CDSPFIRFilter.h"
#include <math.h>
#include <string>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <algorithm>

using namespace Module::Audio;

std::atomic_int Resampler::sId(0);
rpm::map<std::pair<int, int>, int> Resampler::sInLenBeforeOutStart;

Resampler::Resampler(int inRate)
    : mId(sId++),
      mInRate(inRate),
      mOutRate(0)
{
}

Resampler::Resampler(int inRate, int outRate)
    : mId(sId++),
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
        updateRatio();
    }
}

void Resampler::setOutputRate(int newOutRate)
{
    if (mOutRate != newOutRate) {
        mOutRate = newOutRate;
        updateRatio();
    }
}

void Resampler::setRate(int newInRate, int newOutRate)
{
    if (mInRate != newInRate || mOutRate != newOutRate) {
        mInRate = newInRate;
        mOutRate = newOutRate;
        updateRatio();
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
    return mResampler->getLatencyFrac();
}

rpm::vector<double> Resampler::process(const double *pIn, int inLength)
{
    rpm::vector<double> in(&pIn[0], &pIn[inLength]);
    double *op;
    int outLength = mResampler->process(in.data(), inLength, op);

    return rpm::vector<double>(&op[0], &op[outLength]);
}

void Resampler::updateRatio()
{
    setupResampler();
    std::cout << "Audio::Resampler#" << mId << "] Created " << mInRate << " --> " << mOutRate << std::endl;
}

void Resampler::setupResampler()
{
    double resTransBand = (mOutRate < 6000 ? 20.0 : 10.0);

    mResampler = std::make_unique<r8b::CDSPResampler>(mInRate, mOutRate, 65536, resTransBand, 206.91, r8b::fprLinearPhase);

    const int len = getInLenBeforeOutStart(mInRate, mOutRate, *mResampler);
    double *op;
    rpm::vector<double> in(len, 0.0);
    mResampler->process(in.data(), len, op);
}

int Resampler::getInLenBeforeOutStart(int src, int dst, r8b::CDSPResampler& resampler)
{
    auto key = std::make_pair(src, dst);
    auto it = sInLenBeforeOutStart.find(key);
    int inLen;
    if (it != sInLenBeforeOutStart.end()) {
        inLen = it->second;
    }
    else {
        inLen = resampler.getInLenBeforeOutStart();
        sInLenBeforeOutStart[key] = inLen;
    }
    return inLen;
}
