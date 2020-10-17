#include "nodeio_audiotime.h"

using namespace Nodes::IO;

AudioTime::AudioTime()
    : mSampleRate(0), mLength(0)
{
}

void AudioTime::setSampleRate(int sampleRate)
{
    mSampleRate = sampleRate;
}

void AudioTime::setLength(int length)
{
    if (length == mLength) {
        return;
    }

    mData.resize(length);
    mLength = length;
}

double *AudioTime::getData()
{
    return mData.data();
}

int AudioTime::getSampleRate() const
{
    return mSampleRate;
}

int AudioTime::getLength() const
{
    return mLength;
}

const double *AudioTime::getConstData() const
{
    return mData.data();
}
