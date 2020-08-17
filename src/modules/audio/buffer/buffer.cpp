#include "buffer.h"
#include <memory>
#include <iostream>

using namespace Module::Audio;

Buffer::Buffer(int sampleRate, int durationInMs)
    : mSampleRate(sampleRate),
      mLength((sampleRate * durationInMs) / 1000),
      mData(mLength, 0.0f)
{
}

void Buffer::setSampleRate(int newSampleRate)
{
    mLock.lock();
 
    if (mSampleRate == newSampleRate) {
        mLock.unlock();
        return;
    }

    Resampler rsx(mSampleRate, newSampleRate, 1);
   
    int newLength = rsx.getExpectedOutLength(mLength);
    auto newArray = std::make_unique<float[]>(newLength);

    auto array = std::make_unique<float[]>(mLength);
    std::copy(mData.begin(), mData.end(), array.get());

    rsx.process(array.get(), mLength, newArray.get(), newLength);

    mSampleRate = newSampleRate;
    mLength = newLength;
    mData.resize(newLength);
    std::copy(newArray.get(), std::next(newArray.get(), newLength), mData.begin());

    mLock.unlock();
}

void Buffer::setDuration(int newDuration)
{
    setLength((mSampleRate * newDuration) / 1000);
}

void Buffer::setLength(int newLength)
{
    mLock.lock();

    if (mLength == newLength) {
        mLock.unlock();
        return;
    }

    auto array = std::make_unique<float[]>(mLength);
    std::copy(mData.begin(), mData.end(), array.get());
    
    // Copy old buffer to new buffer.

    mData.resize(newLength, 0.0f);

    if (mLength < newLength) {
        std::copy(array.get(), std::next(array.get(), mLength), std::prev(mData.end(), mLength));
    }
    else {
        std::copy(std::next(array.get(), mLength - newLength), std::next(array.get(), mLength), mData.begin());
    }

    mLength = newLength;

    mLock.unlock();
}

int Buffer::getSampleRate() const
{
    return mSampleRate;
}

int Buffer::getDuration() const
{
    return (mLength * 1000) / mSampleRate;
}

int Buffer::getLength() const
{
    return mLength;
}

void Buffer::pull(float *pOut, int outLength)
{
    mLock.lock();

    std::copy(std::prev(mData.end(), outLength), mData.end(), pOut);

    mLock.unlock();
}

void Buffer::push(const float *pIn, int inLength)
{
    mLock.lock();

    mData.insert(mData.end(), pIn, std::next(pIn, inLength));
    mData.erase(mData.begin(), std::next(mData.begin(), inLength));

    mLock.unlock();
}

