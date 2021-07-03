#include "queue.h"
#include <iostream>
#include <memory>

using namespace Module::Audio;

Queue::Queue(int blockDurationInMs, int inSampleRate, QueueCallback callback)
    : mBlockDuration((float) blockDurationInMs),
      mResampler(inSampleRate),
      mCallback(callback),
      mQueue((int) std::round((48000 * 1.5 * mBlockDuration) / 1000))
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
    mResampler.setOutputRate(newOutSampleRate);
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
    int inputRate = mResampler.getInputRate();
    int outputRate = mResampler.getOutputRate();
    int queueSize = (int) mQueue.size_approx();

    float blockDuration = mBlockDuration.load();
    int blockSizeSrc = (int) std::round((blockDuration * inputRate) / 1000);
    int blockSizeDst = (int) std::round((blockDuration * outputRate) / 1000);

    if (queueSize > blockSizeDst) {
        // There's already enough data in the queue.
        return;
    }

    rpm::vector<double> blockSrc(blockSizeSrc);
    mCallback(blockSrc.data(), blockSizeSrc, userdata);
   
    rpm::vector<double> blockDst = mResampler.process(blockSrc);
    for (const double& y : blockDst) {
        mQueue.emplace((float) y);
    }
}

void Queue::pull(float *pOut, int outLength)
{
    bool ranOutOfData = false;

    int i = 0;
    for (; i < outLength; ++i) {
        if (!mQueue.try_dequeue(pOut[i])) {
            ranOutOfData = true;
            break;
        }
    }

    if (ranOutOfData) {
        // Pad the rest with zeroes.
        for (; i < outLength; ++i) {
            pOut[i] = 0.0;
        }
       
        float newBlockDuration = mBlockDuration + 0.25f;
        std::cout << "Audio::Queue] Not enough data to pull. Adjusted block duration to " << newBlockDuration << " ms" << std::endl;
        mBlockDuration = newBlockDuration;
    }
}
