#include "buffer.h"
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>

using namespace Module::Audio;
using namespace std::chrono_literals;

Buffer::Buffer(int sampleRate)
    : mSampleRate(sampleRate),
      mLength(0)
{
}

void Buffer::setSampleRate(int newSampleRate)
{
    std::lock_guard<std::mutex> lock(mMutex);
 
    if (mSampleRate == newSampleRate) {
        return;
    }

    Resampler rsx(mSampleRate, newSampleRate);
   
    int newLength = rsx.getExpectedOutLength(mLength);
    auto newArray = std::make_unique<double[]>(newLength);

    auto array = std::make_unique<double[]>(mLength);
    std::copy(mData.begin(), mData.end(), array.get());

    auto outVec = rsx.process(array.get(), mLength);

    mSampleRate = newSampleRate;
    mLength = outVec.size();
    mData.resize(outVec.size());
    std::copy(outVec.begin(), outVec.end(), mData.begin());
}

int Buffer::getSampleRate() const
{
    return mSampleRate;
}

int Buffer::getLength() const
{
    return mLength;
}

void Buffer::pull(double *pOut, int outLength)
{
    while (mLength.load(std::memory_order_seq_cst) <= outLength) {
        std::this_thread::sleep_for(20ms);
        if (sCancel.load(std::memory_order_relaxed)) {
            return;
        }
    }
    
    mMutex.lock();

    std::copy(mData.begin(), std::next(mData.begin(), outLength), pOut);
    mData.erase(mData.begin(), std::next(mData.begin(), outLength));
    mLength -= outLength;

    mMutex.unlock();
}

void Buffer::push(const float *pIn, int inLength)
{
    mMutex.lock();

    mData.insert(mData.end(), pIn, std::next(pIn, inLength));
    mLength.fetch_add(inLength, std::memory_order_seq_cst);

    mMutex.unlock();
}

std::atomic_bool Buffer::sCancel;

void Buffer::cancelPulls()
{
    sCancel = true;
}
