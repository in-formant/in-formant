#include "resampler.h"
#include <string>
#include <stdexcept>

using namespace Module::Audio;

Resampler::Resampler(int inRate, int outRate, int quality)
    : mInRate(inRate),
      mOutRate(outRate),
      mQuality(quality)
{
    mState = speex_resampler_init(chMono, inRate, outRate, quality, &err);
}

Resampler::~Resampler()
{
    speex_resampler_destroy(mState);
}

void Resampler::setInputRate(int newInRate)
{
    err = speex_resampler_set_rate(mState, newInRate, mOutRate);
    mInRate = newInRate;
    checkError();
}

void Resampler::setOutputRate(int newOutRate)
{
    err = speex_resampler_set_rate(mState, mInRate, newOutRate);
    mOutRate = newOutRate;
    checkError();
}

void Resampler::setRate(int newInRate, int newOutRate)
{
    err = speex_resampler_set_rate(mState, newInRate, newOutRate);
    mInRate = newInRate;
    mOutRate = newOutRate;
    checkError();
}

void Resampler::setQuality(int newQuality)
{
    err = speex_resampler_set_quality(mState, newQuality);
    mQuality = newQuality;
    checkError();
}

int Resampler::getInputRate() const
{
    return mInRate;
}

int Resampler::getOutputRate() const
{
    return mOutRate;
}

void Resampler::getRate(int *pInRate, int *pOutRate) const
{
    *pInRate = mInRate;
    *pOutRate = mOutRate;
}

int Resampler::getQuality() const
{
    return mQuality;
}

int Resampler::getRequiredInLength(int outLength)
{
    uint64_t inLength;
    err = speex_resampler_get_required_input_frame_count(mState, (uint64_t) outLength, &inLength);
    checkError();
    return (int) inLength;
}

int Resampler::getExpectedOutLength(int inLength)
{
    uint64_t outLength;
    err = speex_resampler_get_expected_output_frame_count(mState, (uint64_t) inLength, &outLength);
    checkError();
    return (int) outLength;
}

void Resampler::process(const float *pIn, uint32_t inLength, float *pOut, uint32_t outLength)
{
    constexpr uint32_t framesPerIteration = UINT32_MAX;
    uint32_t framesProcessedOut = 0;
    uint32_t framesProcessedIn = 0;
    
    while (framesProcessedOut < outLength && framesProcessedIn < inLength) {
        uint32_t frameCountInLocal;
        uint32_t frameCountOutLocal;
        const float *pInLocal;
        float *pOutLocal;

        frameCountInLocal = framesPerIteration;
        if (frameCountInLocal > (inLength - framesProcessedIn)) {
            frameCountInLocal = (inLength - framesProcessedIn);
        }

        frameCountOutLocal = framesPerIteration;
        if (frameCountOutLocal > (outLength - framesProcessedOut)) {
            frameCountOutLocal = (outLength - framesProcessedOut);
        }

        pInLocal = pIn + framesProcessedIn * sizeof(float);
        pOutLocal = pOut + framesProcessedOut * sizeof(float);

        err = speex_resampler_process_interleaved_float(mState, pInLocal, &frameCountInLocal, pOutLocal, &frameCountOutLocal);
        checkError();

        framesProcessedIn += frameCountInLocal;
        framesProcessedOut += frameCountOutLocal;
    }
}

void Resampler::checkError()
{
    if (err > 0) {
        throw std::runtime_error(std::string("Audio::Resampler] ") + speex_resampler_strerror(err));
    }
}
