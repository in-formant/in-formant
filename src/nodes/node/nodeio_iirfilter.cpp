#include "nodeio_iirfilter.h"

using namespace Nodes::IO;

IIRFilter::IIRFilter()
    : mSampleRate(0),
      mFFOrder(0), mFBOrder(0),
      mFF(nullptr), mFB(nullptr)
{
}

IIRFilter::~IIRFilter()
{
    delete[] mFF;
    delete[] mFB;
}

void IIRFilter::setSampleRate(int sampleRate)
{
    mSampleRate = sampleRate;
}

void IIRFilter::setFFOrder(int order)
{
    if (order == mFFOrder) {
        return;
    }

    delete[] mFF;
    mFF = new float[order];
    mFFOrder = order;
}

void IIRFilter::setFBOrder(int order)
{
    if (order == mFBOrder) {
        return;
    }

    delete[] mFB;
    mFB = new float[order];
    mFBOrder = order;
}

float *IIRFilter::getFFData()
{
    return mFF;
}

float *IIRFilter::getFBData()
{
    return mFB;
}

int IIRFilter::getSampleRate() const
{
    return mSampleRate;
}

int IIRFilter::getFFOrder() const
{
    return mFFOrder;
}

int IIRFilter::getFBOrder() const
{
    return mFBOrder;
}

const float *IIRFilter::getFFConstData() const
{
    return mFF;
}

const float *IIRFilter::getFBConstData() const
{
    return mFB;
}
