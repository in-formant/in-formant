#include "queue.h"
#include <iostream>
#include <memory>

using namespace Module::Audio;

Queue::Queue(int minInDurationInMs, int maxInDurationInMs, int avgDurationInMs, int inSampleRate, QueueCallback callback)
    : mMinInLength((inSampleRate * minInDurationInMs) / 1000),
      mMaxInLength((inSampleRate * maxInDurationInMs) / 1000),
      mAvgLength((inSampleRate * avgDurationInMs) / 1000),
      mResampler(inSampleRate, 8000),
      mCallback(callback)
{
}

Queue::~Queue()
{
}

void Queue::setCallback(QueueCallback callback)
{
    mCallback = callback;
}

void Queue::setOutSampleRate(int newOutSampleRate)
{
    mLock.lock();
    mResampler.setOutputRate(newOutSampleRate);
    mLock.unlock();
}

int Queue::getInSampleRate() const
{
    return mResampler.getInputRate();
}

int Queue::getOutSampleRate() const
{
    return mResampler.getOutputRate();
}

void Queue::pushIfNeeded(void *userdata)
{
    mLock.lock();

    int pushSize = mAvgLength - mDeque.size();
    if (pushSize < mMinInLength) {
        mLock.unlock();
        return;
    }
    
    if (pushSize > mMaxInLength) {
        pushSize = mMaxInLength;
    }
    
    auto inputArray = std::make_unique<double[]>(pushSize);
    
    mCallback(inputArray.get(), pushSize, userdata);
    
    mDeque.insert(mDeque.end(), inputArray.get(), std::next(inputArray.get(), pushSize));

    mLock.unlock();
}

void Queue::pull(float *pOut, int outLength)
{
    mLock.lock();
  
    int pullSize = mResampler.getRequiredInLength(outLength);

    auto preResampleData = std::make_unique<double[]>(pullSize);

    int pullCopyLength;
    if (pullSize > mDeque.size()) {
        //std::cout << "Audio::Queue] not enough data to pull, padding with zeroes" << std::endl;
        pullCopyLength = mDeque.size();
    }
    else {
        pullCopyLength = pullSize;
    }

    auto pullEndIt = std::next(mDeque.begin(), pullCopyLength);

    std::copy(mDeque.begin(), pullEndIt, preResampleData.get());
    std::fill(preResampleData.get() + pullCopyLength, std::next(preResampleData.get(), pullSize), 0.0f);

    auto outVec = mResampler.process(preResampleData.get(), pullSize);
    std::copy(outVec.begin(), outVec.end(), pOut);

    mDeque.erase(mDeque.begin(), pullEndIt);

    mLock.unlock();
}
