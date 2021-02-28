#include "buffer.h"
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>

using namespace Module::Audio;
using namespace std::chrono_literals;

std::atomic_bool Buffer::sCancel(false);
std::atomic_int Buffer::sId(0);

Buffer::Buffer(double sampleRate)
    : mId(sId++),
      mSampleRate(sampleRate),
      mQueue(1024)
{
}

void Buffer::setSampleRate(double newSampleRate)
{
    if (mSampleRate == newSampleRate) {
        return;
    }
    else {
        if (mSampleRate > 0) {
            std::cout << "Audio::Buffer#" << mId << "] Sample rate changed, might cause jank for the next few frames." << std::endl;
        }
        mSampleRate = newSampleRate;
    }
}

double Buffer::getSampleRate() const
{
    return mSampleRate;
}

int Buffer::getLength() const
{
    return mQueue.size_approx();
}

void Buffer::pull(double *pOut, int outLength)
{
    for (int i = 0; i < outLength; ++i) {
        while (!mQueue.wait_dequeue_timed(pOut[i], 50) && !sCancel);
    }
}

void Buffer::push(const float *pIn, int inLength)
{
    for (int i = 0; i < inLength; ++i) {
        mQueue.enqueue(pIn[i]);
    }
}

void Buffer::cancelPulls()
{
    sCancel = true;
}
