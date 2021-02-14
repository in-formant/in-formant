#include "qpainterwrapper.h"
#include <gaborator/gaussian.h>
#include <cmath>
#include <iostream>
#include <mutex>

constexpr double mapToUnit(double v, double min, double max) {
    return (v - min) / (max - min);
}

inline double hz2mel(double f) {
    return 2595.0 * log10(1.0 + f / 700.0);
}

inline double mel2hz(double m) {
    return 700.0 * (pow(10.0, m / 2595.0) - 1.0);
}

inline double hz2log(double f) {
    return log2(f);
}

inline double log2hz(double l) {
    return exp2(l);
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

QRgb QPainterWrapper::mapAmplitudeToColor(double amplitude, double minGain, double maxGain)
{
    /*
    double gain = amplitude > 1e-10 ? 20 * log10(amplitude) : -1e6;
    double clampedGain = std::clamp(gain, minGain, maxGain);

    double clampedAmplitude = pow(10.0, clampedGain / 20);
    */

    double a = sqrt(amplitude) * 7.5;

    int leftIndex = std::floor(a * 255);

    if (leftIndex <= 0) {
        return cmap[0];
    }
    else if (leftIndex >= 255) {
        return cmap[255];
    }
    else {
        double frac = a * 255 - leftIndex;
        QColor c1 = QColor::fromRgb(cmap[leftIndex]);
        QColor c2 = QColor::fromRgb(cmap[leftIndex + 1]);
        int r = frac * c1.red() + (1 - frac) * c2.red();
        int g = frac * c1.green() + (1 - frac) * c2.green();
        int b = frac * c1.blue() + (1 - frac) * c2.blue();
        return qRgb(r, g, b);
    }
}
// ( vh, h, scale, minf, maxf )
using ytrans_key = std::tuple<int, int, QPainterWrapper::FrequencyScale, double, double>;

static rpm::map<ytrans_key, Eigen::MatrixXd> ytrans_map;

static std::mutex ytrans_mutex;

Eigen::MatrixXd QPainterWrapper::constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax)
{
    std::lock_guard<std::mutex> ytrans_lock(ytrans_mutex);

    auto key = std::make_tuple(vh, h, freqScale, freqMin, freqMax);
    auto it = ytrans_map.find(key);
    if (it != ytrans_map.end()) {
        return it->second;
    }

    const double lmin = hz2log(freqMin);
    const double lmax = hz2log(freqMax); 

    Eigen::MatrixXd ytrans(h, vh);
    ytrans.setZero();

    const double sd = (exp2(1.0 / 24) - 1) * 1000.0;

    for (int y = 0; y < h; ++y) {
        const double fc = log2hz(lmin + (1 - (double) y / (double) h) * (lmax - lmin));

        const double vyc = mapFrequencyToY(fc, vh, freqScale, freqMin, freqMax);
        
        for (int vy = 0; vy < vh; ++vy) {
            ytrans(y, vy) = gaborator::norm_gaussian(sd, (vy - vyc) * 8);
        }
    }

    ytrans_map.emplace(key, ytrans);
    
    return ytrans;
}

QImage QPainterWrapper::drawSpectrogram(const rpm::vector<double>& amplitudes, int width, int height, int viewportWidth, int viewportHeight, FrequencyScale scale, double minFrequency, double maxFrequency, double minGain, double maxGain)
{
    Eigen::Map<const Eigen::MatrixXd> logSpec(amplitudes.data(), width, height);
    
    Eigen::MatrixXd ytrans = constructTransformY(height, viewportHeight, scale, minFrequency, maxFrequency);
    
    QImage image(width, viewportHeight, QImage::Format_RGB32);
    
    const int chunkWidth = 50;
    int x = 0;
    
    while (x + chunkWidth < width) {
        auto chunk = logSpec.block(x, 0, chunkWidth, height);

        Eigen::MatrixXd mappedChunk = (chunk * ytrans).transpose();

        for (int vy = 0; vy < mappedChunk.rows(); ++vy) {
            QRgb *scanLineBits = reinterpret_cast<QRgb *>(image.scanLine(vy));
            for (int vx = 0; vx < mappedChunk.cols(); ++vx) {
                scanLineBits[x + vx] = mapAmplitudeToColor(mappedChunk(vy, vx), minGain, maxGain);
            }
        }

        x += chunkWidth;
    }

    if (x < width) {
        auto chunk = logSpec.block(x, 0, width - x, height);

        Eigen::MatrixXd mappedChunk = (chunk * ytrans).transpose();

        for (int vy = 0; vy < mappedChunk.rows(); ++vy) {
            QRgb *scanLineBits = reinterpret_cast<QRgb *>(image.scanLine(vy));
            for (int vx = 0; vx < mappedChunk.cols(); ++vx) {
                scanLineBits[x + vx] = mapAmplitudeToColor(mappedChunk(vy, vx), minGain, maxGain);
            }
        }
    }

    return image.scaled(viewportWidth, viewportHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
