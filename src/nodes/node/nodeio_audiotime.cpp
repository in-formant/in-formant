#include "nodeio_audiotime.h"

using namespace Nodes::IO;

AudioTime::AudioTime()
    : mSampleRate(0), mLength(0), mData(nullptr)
{
}

AudioTime::~AudioTime()
{
    delete[] mData;
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

    delete[] mData;
    mData = new float[length];
    mLength = length;
}

float *AudioTime::getData()
{
    return mData;
}

int AudioTime::getSampleRate() const
{
    return mSampleRate;
}

int AudioTime::getLength() const
{
    return mLength;
}

const float *AudioTime::getConstData() const
{
    return mData;
}
