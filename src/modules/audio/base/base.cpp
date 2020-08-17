#include "base.h"

using namespace Module::Audio;

AbstractBase::AbstractBase()
{
}

AbstractBase::~AbstractBase()
{
}

void AbstractBase::tickAudio()
{
    if (needsTicking()) {
        throw std::runtime_error("Audio::AbstractBase] tickAudio needs to be implemented for ticking audio backends");
    }
}

void AbstractBase::setCaptureBuffer(Buffer *buffer)
{
    mCaptureBuffer = buffer;
}

void AbstractBase::setPlaybackQueue(Queue *queue)
{
    mPlaybackQueue = queue;
}

void AbstractBase::setCaptureBufferSampleRate(int sampleRate)
{
    mCaptureBuffer->setSampleRate(sampleRate);
}

void AbstractBase::pushToCaptureBuffer(const float *data, int length)
{
    mCaptureBuffer->push(data, length);
}
