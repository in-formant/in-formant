#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "base.h"

#include <map>
#include <utility>
#include <cmath>

using namespace Module::Renderer;

AbstractBase::AbstractBase(Type type)
    : mType(type),
      mDrawableWidth(1280),
      mDrawableHeight(720),
      mDrawableSizeChanged(false),
      mWindowWidth(1280),
      mWindowHeight(720),
      mWindowSizeChanged(false),
      mParameters(new Parameters)
{
}

AbstractBase::~AbstractBase()
{
    delete mParameters;
}

void AbstractBase::setDrawableSize(int width, int height)
{
    mDrawableWidth = width;
    mDrawableHeight = height;
    mDrawableSizeChanged = true;
}

void AbstractBase::getDrawableSize(int *pWidth, int *pHeight) const
{
    *pWidth = mDrawableWidth;
    *pHeight = mDrawableHeight;
}

bool AbstractBase::hasDrawableSizeChanged() const
{
    return mDrawableSizeChanged;
}

void AbstractBase::resetDrawableSizeChanged()
{
    mDrawableSizeChanged = false;
}

void AbstractBase::setWindowSize(int width, int height)
{
    mWindowWidth = width;
    mWindowHeight = height;
    mWindowSizeChanged = true;
}

void AbstractBase::getWindowSize(int *pWidth, int *pHeight) const
{
    *pWidth = mWindowWidth;
    *pHeight = mWindowHeight;
}

bool AbstractBase::hasWindowSizeChanged() const
{
    return mWindowSizeChanged;
}

void AbstractBase::resetWindowSizeChanged()
{
    mWindowSizeChanged = false;
}

Parameters *AbstractBase::getParameters()
{
    return mParameters;
}

float AbstractBase::frequencyToCoordinate(float frequency) const
{
    static std::map<
            std::tuple<FrequencyScale, float, float>,
            std::map<float, float>> mappings;

    FrequencyScale scale = mParameters->getFrequencyScale();
    float minFrequency = mParameters->getMinFrequency();
    float maxFrequency = mParameters->getMaxFrequency();

    float transMin, transMax, transVal;
    
    auto key = std::make_tuple(scale, minFrequency, maxFrequency);
    auto it1 = mappings.find(key);
    if (it1 == mappings.end()) {
        mappings.emplace(key, std::map<float, float>());
        it1 = mappings.find(key);
    }

    auto& map = it1->second;

    auto it2 = map.find(frequency);
    if (it2 != map.end()) {
        return it2->second;
    }
    else {
        switch (scale) {
        case FrequencyScale::Linear:
            transMin = minFrequency;
            transMax = maxFrequency;
            transVal = frequency;
            break;
        case FrequencyScale::Logarithmic:
            transMin = log10f(10.0f + minFrequency);
            transMax = log10f(10.0f + maxFrequency);
            transVal = log10f(10.0f + frequency);
            break;
        case FrequencyScale::Mel:
            transMin = 2595.0f * log10f(1.0f + minFrequency / 700.0f);
            transMax = 2595.0f * log10f(1.0f + maxFrequency / 700.0f);
            transVal = 2595.0f * log10f(1.0f + frequency / 700.0f);
            break;
        }

        float coord = 2.0f * (transVal - transMin) / (transMax - transMin) - 1.0f;
        map.emplace(frequency, coord);
        return coord;
    }
}

float AbstractBase::coordinateToFrequency(float y) const
{
    static std::map<
            std::tuple<FrequencyScale, float, float>,
            std::map<float, float>> mappings;

    FrequencyScale scale = mParameters->getFrequencyScale();
    float minFrequency = mParameters->getMinFrequency();
    float maxFrequency = mParameters->getMaxFrequency();

    float transMin, transMax, transVal;
    
    auto key = std::make_tuple(scale, minFrequency, maxFrequency);
    auto it1 = mappings.find(key);
    if (it1 == mappings.end()) {
        mappings.emplace(key, std::map<float, float>());
        it1 = mappings.find(key);
    }

    auto& map = it1->second;

    auto it2 = map.find(y);
    if (it2 != map.end()) {
        return it2->second;
    }
    else {
        switch (scale) {
        case FrequencyScale::Linear:
            transMin = minFrequency;
            transMax = maxFrequency;
            break;
        case FrequencyScale::Logarithmic:
            transMin = log10f(10.0f + minFrequency);
            transMax = log10f(10.0f + maxFrequency);
            break;
        case FrequencyScale::Mel:
            transMin = 2595.0f * log10f(1.0f + minFrequency / 700.0f);
            transMax = 2595.0f * log10f(1.0f + maxFrequency / 700.0f);
            break;
        }

        float coord = 2.0f * (transVal - transMin) / (transMax - transMin) - 1.0f;

        transVal = (y + 1.0f) / 2.0f * (transMax - transMin) + transMin;

        float frequency;
        switch (scale) {
        case FrequencyScale::Linear:
            frequency = transVal;
            break;
        case FrequencyScale::Logarithmic:
            frequency = powf(10.0f, transVal) - 10.0f;
            break;
        case FrequencyScale::Mel:
            frequency = 700.0f * (powf(10.0f, transVal / 2595.0f) - 1.0f);
            break;
        }

        map.emplace(y, frequency);
        return frequency;
    }
}


void AbstractBase::gainToColor(float gain, float *r, float *g, float *b) const
{
    float minGain = mParameters->getMinGain();
    float maxGain = mParameters->getMaxGain();

    float a = clamp((gain - minGain) / (maxGain - minGain), 0.0f, 1.0f);

    *r = 5.0f * (a - 0.2f);
    *g = 5.0f * (a - 0.6f);

    if (a > 0.8)      *b = 5.0f * (a - 0.8f);
    else if (a > 0.4) *b = 5.0f * (0.6f - a);
    else              *b = 5.0f * a;

    *r = clamp(*r, 0.0f, 0.98f);
    *g = clamp(*g, 0.0f, 0.98f);
    *b = clamp(*b, 0.0f, 0.98f);
}
