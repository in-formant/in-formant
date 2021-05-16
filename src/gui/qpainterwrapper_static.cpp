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

// ( vh, h, scale, minf, maxf, scalesrc, minsrc, maxsrc )
using ytrans_key = std::tuple<int, int, FrequencyScale, double, double, FrequencyScale, double, double>;

static rpm::map<ytrans_key, Eigen::SparseMatrix<double>> ytrans_map;

static QMutex ytrans_mutex;

Eigen::SparseMatrix<double>& QPainterWrapper::constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax, FrequencyScale sourceScale, double sourceMin, double sourceMax)
{
    QMutexLocker ytrans_lock(&ytrans_mutex);

    auto key = std::make_tuple(vh, h, freqScale, freqMin, freqMax, sourceScale, sourceMin, sourceMax);
    auto it = ytrans_map.find(key);
    if (it != ytrans_map.end()) {
        return it->second;
    }

    Eigen::SparseMatrix<double> ytrans;

    if (freqScale == FrequencyScale::Linear) {
        ytrans = Analysis::linearFilterbank(freqMin, freqMax, vh, h, 2 * sourceMax);
    }
    else if (freqScale == FrequencyScale::Logarithmic) {
        ytrans = Analysis::logFilterbank(freqMin, freqMax, vh, h, 2 * sourceMax);
    }
    else if (freqScale == FrequencyScale::Mel) {
        ytrans = Analysis::melFilterbank(freqMin, freqMax, vh, h, 2 * sourceMax);
    }
    else if (freqScale == FrequencyScale::ERB) {
        ytrans = Analysis::erbFilterbank(freqMin, freqMax, vh, h, 2 * sourceMax);
    }

    if (ytrans.size() > 0) {
        ytrans_map.emplace(key, ytrans);
    }
    else {
        std::cout << "!! Y axis transform was not supported" << std::endl;
        return ytrans_map.begin()->second;
    }
    
    return ytrans_map[key];
}

static rpm::map<std::tuple<double, FrequencyScale, double, double>, Eigen::VectorXd> maps;
static constexpr size_t mapsMaxSize = 1024UL * 1024UL * 128UL;

void QPainterWrapper::drawSpectrogram(
            const rpm::vector<std::pair<double, Main::SpectrogramCoefs>>& slices)
{
    // Divide the sequence of spectrogram slices into multiple sequences of similar slices.

    int lastNfft = 0;
    double lastSampleRate = 0;
    double lastFrameDuration = 0;

    rpm::vector<rpm::vector<std::pair<double, Main::SpectrogramCoefs>>> segments;
    rpm::vector<std::pair<double, Main::SpectrogramCoefs>> currentSegment;

    for (const auto& slice : slices) {
        if ((slice.second.magnitudes.size() != lastNfft
                    || slice.second.sampleRate != lastSampleRate
                    || slice.second.frameDuration != lastFrameDuration)
                && !currentSegment.empty()) {
            segments.push_back(std::move(currentSegment));
            lastNfft = slice.second.magnitudes.size();
            lastSampleRate = slice.second.sampleRate;
            lastFrameDuration = slice.second.frameDuration;
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
        const double frameDuration = segment[0].second.frameDuration;
    
        const int segmentLen = segment.size();

        segmentTexture.resize(nfft * segmentLen);
        for (int y = 0; y < nfft; ++y) {
            for (int x = 0; x < segmentLen; ++x) {
                segmentTexture[y * segmentLen + x] = segment[x].second.magnitudes(y);
            }
        }
    
        p->prepareSpectrogramDraw(cmap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, segmentLen, nfft, 0, GL_RED, GL_FLOAT, segmentTexture.data());
        p->drawSpectrogram(
                sampleRate / 2,
                mFrequencyScale,
                mMinFrequency, mMaxFrequency,
                mapTimeToX(segment.front().first - frameDuration / 2),
                mapTimeToX(segment.back().first + frameDuration / 2));
    }
}

