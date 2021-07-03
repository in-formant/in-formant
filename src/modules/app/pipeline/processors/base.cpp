#include "base.h"
#include "../../../../context/contextmanager.h"
#include <exception>

using namespace Module::App::Processors;

BaseProcessor::BaseProcessor(double frameSpace, double frameLength)
    : mFrameSpace(frameSpace / 1000.0),
      mFrameLength(frameLength / 1000.0),
      mTime(0)
{
}

bool BaseProcessor::canProcess(double timeNow) const
{
    return timeNow - mTime >= mFrameSpace;
}

void BaseProcessor::process(const rpm::vector<double>& slidingWindow, double sampleRate, double timeNow)
{
    const int frameSamples = (int) std::round(mFrameLength * sampleRate);

    mData.resize(frameSamples);

    std::copy(std::prev(slidingWindow.end(), frameSamples), slidingWindow.end(),
            mData.begin());

#ifdef _WIN32
    try {
        processData(mData, sampleRate);
    }
    catch (const std::exception& e) {
        StdExceptionHandler(e);
    }
#else
    processData(mData, sampleRate);
#endif

    mTime = timeNow;
}

double BaseProcessor::getFrameSpace() const
{
    return mFrameSpace;
}

double BaseProcessor::getFrameLength() const
{
    return mFrameLength;
}

double BaseProcessor::getCenteredTime() const
{
    return mTime + mFrameLength / 2;
}