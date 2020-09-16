#include "nodeio_audiospec.h"

using namespace Nodes::IO;

AudioSpec::AudioSpec()
    : mSampleRate(0), mLength(0)
{
}

void AudioSpec::setSampleRate(int sampleRate)
{
    mSampleRate = sampleRate;
}

void AudioSpec::setLength(int length)
{
    if (length == mLength) {
        return;
    }

    mData.resize(length);
    mLength = length;
}

float *AudioSpec::getData()
{
    return mData.data();
}

int AudioSpec::getSampleRate() const
{
    return mSampleRate;
}

int AudioSpec::getLength() const
{
    return mLength;
}

const float *AudioSpec::getConstData() const
{
    return mData.data();
}

