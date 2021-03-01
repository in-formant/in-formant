#include "qpainterwrapper.h"
#include <cmath>
#include <iostream>
#include <mutex>

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

QRgb QPainterWrapper::mapAmplitudeToColor(double amplitude, double minGain, double maxGain)
{
    //double a = std::clamp((gain - minGain) / (maxGain - minGain), 0.0, 1.0);
    double a = amplitude / pow(10, maxGain / 20);
    a = sqrt(a) * 7;
    
    int leftIndex = std::floor(a * 255);

    QColor c;

    if (a < 1e-16) {
        return QColor(Qt::transparent).rgba();
    }

    if (leftIndex <= 0) {
        c = QColor::fromRgb(cmap[0]);
    }
    else if (leftIndex >= 255) {
        c = QColor::fromRgb(cmap[255]);
    }
    else {
        double frac = a * 255 - leftIndex;
        QColor c1 = QColor::fromRgb(cmap[leftIndex]);
        QColor c2 = QColor::fromRgb(cmap[leftIndex + 1]);
        int r = frac * c1.red() + (1 - frac) * c2.red();
        int g = frac * c1.green() + (1 - frac) * c2.green();
        int b = frac * c1.blue() + (1 - frac) * c2.blue();
        c = QColor::fromRgb(qRgb(r, g, b));
    }

    return c.rgba();
}

// ( vh, h, scale, minf, maxf, scalesrc, minsrc, maxsrc )
using ytrans_key = std::tuple<int, int, FrequencyScale, double, double, FrequencyScale, double, double>;

static rpm::map<ytrans_key, Eigen::SparseMatrix<double>> ytrans_map;

static std::mutex ytrans_mutex;

Eigen::SparseMatrix<double> QPainterWrapper::constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax, FrequencyScale sourceScale, double sourceMin, double sourceMax)
{
    std::lock_guard<std::mutex> ytrans_lock(ytrans_mutex);

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
    }
    
    return ytrans;
}

