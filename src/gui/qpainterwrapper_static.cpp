#include "qpainterwrapper.h"
#include "../context/timings.h"
#include <cmath>
#include <iostream>
#include <QMutex>

constexpr double mapToUnit(double v, double min, double max) {
    return (v - min) / (max - min);
}

double QPainterWrapper::transformFrequency(double frequency, FrequencyScale scale)
{
    switch (scale) {
    case FrequencyScale::Linear:
        return frequency;
    case FrequencyScale::Logarithmic:
        return hz2log(frequency);
    case FrequencyScale::Mel:
        return hz2mel(frequency);
    case FrequencyScale::ERB:
        return hz2erb(frequency);
    default:
        return 0;
    }
}

double QPainterWrapper::inverseFrequency(double value, FrequencyScale scale)
{
    switch (scale) {
    case FrequencyScale::Linear:
        return value;
    case FrequencyScale::Logarithmic:
        return log2hz(value);
    case FrequencyScale::Mel:
        return mel2hz(value);
    case FrequencyScale::ERB:
        return erb2hz(value);
    default:
        return 0;
    }
}

double QPainterWrapper::mapTimeToX(double time, int width, double startTime, double endTime)
{
    return width * (time - startTime) / (endTime - startTime);
}

double QPainterWrapper::mapFrequencyToY(double frequency, int height, FrequencyScale scale, double minFrequency, double maxFrequency)
{
    const double vmin = transformFrequency(minFrequency, scale);
    const double vmax = transformFrequency(maxFrequency, scale);
    const double value = transformFrequency(frequency, scale);

    return height * (1 - (value - vmin) / (vmax - vmin));
}

double QPainterWrapper::mapYToFrequency(double y, int height, FrequencyScale scale, double minFrequency, double maxFrequency)
{
    const double vmin = transformFrequency(minFrequency, scale);
    const double vmax = transformFrequency(maxFrequency, scale);

    const double value = vmin + (1 - y / (double) height) * (vmax - vmin);

    return inverseFrequency(value, scale);
}

void QPainterWrapper::drawSpectrogram(
            const rpm::vector<std::pair<double, Main::SpectrogramCoefs>>& slices)
{
    // Divide the sequence of spectrogram slices into multiple sequences of similar slices.

    int lastNfft = 0;
    double lastSampleRate = 0;

    rpm::vector<rpm::vector<std::pair<double, Main::SpectrogramCoefs>>> segments;
    rpm::vector<std::pair<double, Main::SpectrogramCoefs>> currentSegment;

    for (const auto& slice : slices) {
        if ((slice.second.magnitudes.size() != lastNfft
                    || slice.second.sampleRate != lastSampleRate)
                && !currentSegment.empty()) {
            segments.push_back(std::move(currentSegment));
            currentSegment.clear();
            lastNfft = slice.second.magnitudes.size();
            lastSampleRate = slice.second.sampleRate;
        }
        currentSegment.push_back(slice);
    }

    if (!currentSegment.empty()) {
        segments.push_back(std::move(currentSegment));
    }
   
    static rpm::vector<GLfloat> segmentTexture;

    for (const auto& segment : segments) {
        const int nfft = segment[0].second.magnitudes.size();
        const double sampleRate = segment[0].second.sampleRate;
    
        const int segmentLen = segment.size();

        segmentTexture.resize(nfft * segmentLen);
        for (int y = 0; y < nfft; ++y) {
            for (int x = 0; x < segmentLen; ++x) {
                segmentTexture[y * segmentLen + x] = segment[x].second.magnitudes(y);
            }
        }
    
        p->prepareSpectrogramDraw();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, segmentLen, nfft, 0, GL_RED, GL_FLOAT, segmentTexture.data());
        p->drawSpectrogram(
                sampleRate / 2,
                mFrequencyScale,
                mMinFrequency, mMaxFrequency,
                mMaxGain,
                cmap,
                mapTimeToX(segment.front().first),
                mapTimeToX(segment.back().first),
                mapFrequencyToY(mMinFrequency),
                mapFrequencyToY(mMaxFrequency));
    }
}

