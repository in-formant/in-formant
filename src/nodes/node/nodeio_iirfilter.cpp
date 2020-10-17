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
    mFF = new double[order];
    mFFOrder = order;
}

void IIRFilter::setFBOrder(int order)
{
    if (order == mFBOrder) {
        return;
    }

    delete[] mFB;
    mFB = new double[order];
    mFBOrder = order;
}

double *IIRFilter::getFFData()
{
    return mFF;
}

double *IIRFilter::getFBData()
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

const double *IIRFilter::getFFConstData() const
{
    return mFF;
}

const double *IIRFilter::getFBConstData() const
{
    return mFB;
}
