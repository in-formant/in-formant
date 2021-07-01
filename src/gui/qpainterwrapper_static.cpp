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

void QPainterWrapper::drawSpectrogram(const rpm::vector<std::pair<double, Main::SpectrogramCoefs>>& slices)
{
    if (slices.empty()) {
        return;
    }

    // The maximum number of slices without program modifications is ~1500.
    // Rounded to the higher power of two that's 2048.
    
    constexpr int texWidth = 2048;
    constexpr int texHeight = 4096;

    static int xOffset = 0;

    static std::array<GLint, texWidth> nffts;
    static std::array<GLfloat, texWidth> sampleRates;

    // Only render the slices that have not been rendered yet. 

    static double lastTimeEnd = -HUGE_VAL; 

    int firstSliceIndexToRender = 0;

    while (firstSliceIndexToRender < slices.size()
                && slices[firstSliceIndexToRender].first <= lastTimeEnd) {
        ++firstSliceIndexToRender;
    }

    lastTimeEnd = slices.back().first;

    const int sliceCount = (int) slices.size() - firstSliceIndexToRender;

    if (sliceCount > 0) {
        // Split it into two chunks if it overlaps the texture width.
        int sliceCount1, sliceCount2;
        if ((xOffset % texWidth) + sliceCount > texWidth) {
            sliceCount1 = texWidth - (xOffset % texWidth);
            sliceCount2 = sliceCount - sliceCount1;
        }
        else {
            sliceCount1 = sliceCount;
            sliceCount2 = 0;
        }

        static rpm::vector<GLfloat> data1, data2;

        data1.resize(sliceCount1 * texHeight);
        data2.resize(sliceCount2 * texHeight);

        std::fill(data1.begin(), data1.end(), 0.0);
        std::fill(data2.begin(), data2.end(), 0.0);

        for (int ioff = 0; ioff < sliceCount; ++ioff) {
            const auto& slice = slices[firstSliceIndexToRender + ioff].second;
            const auto& fftData = slice.magnitudes;
            const int nfft = fftData.size();
            const double sampleRate = slice.sampleRate;

            int index;
            if (ioff < sliceCount1) {
                index = (xOffset % texWidth) + ioff;
            }
            else {
                index = ioff;
            }

            nffts[index] = nfft;
            sampleRates[index] = sampleRate;

            for (int k = 0; k < nfft; ++k) {
                if (ioff < sliceCount1) {
                    data1[k * sliceCount + ioff] = fftData(k);
                }
                else {
                    data2[k * sliceCount + ioff] = fftData(k);
                }
            }
        }

        p->drawSpectrogram(
            xOffset,
            sliceCount1,
            sliceCount2,
            slices.size(),
            nffts,
            sampleRates,
            data1,
            data2,
            mFrequencyScale,
            mMinFrequency, mMaxFrequency,
            mMaxGain,
            cmap,
            slices.front().first,
            slices.back().first,
            mTimeStart,
            mTimeEnd);
        
        xOffset += sliceCount;
    }
    else {
        static rpm::vector<GLfloat> emptyData(0);

        p->drawSpectrogram(
            xOffset,
            0,
            0,
            slices.size(),
            nffts,
            sampleRates,
            emptyData,
            emptyData,
            mFrequencyScale,
            mMinFrequency, mMaxFrequency,
            mMaxGain,
            cmap,
            slices.front().first,
            slices.back().first,
            mTimeStart,
            mTimeEnd);
    }
}

