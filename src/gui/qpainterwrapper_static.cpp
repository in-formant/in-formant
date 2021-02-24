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

inline double hz2erb(double f) {
    constexpr double A = 21.33228113095401739888262;
    return A * log10(1 + 0.00437 * f);
}

inline double erb2hz(double erb) {
    constexpr double A = 21.33228113095401739888262;
    return (pow(10.0, erb / A) - 1) / 0.00437;
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

QRgb QPainterWrapper::mapAmplitudeToColor(double amplitude, double minGain, double maxGain)
{
    double gain = amplitude > 1e-10 ? 20.0 * log10(amplitude) : -1e6;
    double clampedGain = std::clamp(gain, minGain, maxGain);
    double a = pow(10.0, clampedGain / 20.0);
    
    a = sqrt(amplitude) * 30;

    int leftIndex = std::floor(a * 255);

    QColor c;

    if (a < 1e-6) {
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

// ( vh, h, scale, minf, maxf, minsrc, maxsrc )
using ytrans_key = std::tuple<int, int, QPainterWrapper::FrequencyScale, double, double, double, double>;

static rpm::map<ytrans_key, Eigen::SparseMatrix<double>> ytrans_map;

static std::mutex ytrans_mutex;

Eigen::SparseMatrix<double> QPainterWrapper::constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax, double sourceMin, double sourceMax)
{
    std::lock_guard<std::mutex> ytrans_lock(ytrans_mutex);

    auto key = std::make_tuple(vh, h, freqScale, freqMin, freqMax, sourceMin, sourceMax);
    auto it = ytrans_map.find(key);
    if (it != ytrans_map.end()) {
        return it->second;
    }

    const double lmin = hz2log(sourceMin);
    const double lmax = hz2log(sourceMax); 

    Eigen::SparseMatrix<double> ytrans(h, vh);
    ytrans.setZero();

    const double sd = (exp2(1.0 / 48) - 1) * 1000.0;

    for (int y = 0; y < h; ++y) {
        const double fc = log2hz(lmin + (1 - (double) y / (double) h) * (lmax - lmin));
        const double f1 = log2hz(lmin + (1 - (double) (y+1) / (double) h) * (lmax - lmin));
        const double f2 = log2hz(lmin + (1 - (double) (y-1) / (double) h) * (lmax - lmin));

        const double vyc = mapFrequencyToY(fc, vh, freqScale, freqMin, freqMax);
        const double vy1 = mapFrequencyToY(f1, vh, freqScale, freqMin, freqMax);
        const double vy2 = mapFrequencyToY(f2, vh, freqScale, freqMin, freqMax);

        int vy1real = std::floor(std::min(vy1, vy2)) - 1;
        int vy2real = std::ceil(std::max(vy1, vy2)) + 1;

        for (int vy = vy1real; vy <= vy2real; ++vy) {
            if (vy >= 0 && vy < vh) {
                // Triangular shape weight
                ytrans.insert(y, vy) = 1.0 - fabs(vy - vyc) / (double) (vy2real - vy1real);
            }
        }
    }

    ytrans_map.emplace(key, ytrans);
    
    return ytrans;
}

QImage QPainterWrapper::drawSpectrogram(const rpm::vector<double>& amplitudes, double sourceMin, double sourceMax, int width, int height, int viewportWidth, int viewportHeight, FrequencyScale scale, double minFrequency, double maxFrequency, double minGain, double maxGain)
{
    Eigen::Map<const Eigen::MatrixXd> logSpec(amplitudes.data(), width, height);
    
    Eigen::SparseMatrix<double> ytrans = constructTransformY(height, viewportHeight, scale, minFrequency, maxFrequency, sourceMin, sourceMax);
    
    QImage image(width, viewportHeight, QImage::Format_ARGB32_Premultiplied);
    
    Eigen::MatrixXd mapped = (logSpec * ytrans).transpose();
    for (int vy = 0; vy < mapped.rows(); ++vy) {
        QRgb *scanLineBits = reinterpret_cast<QRgb *>(image.scanLine(vy));
        for (int vx = 0; vx < mapped.cols(); ++vx) {
            scanLineBits[vx] = mapAmplitudeToColor(mapped(vy, vx), minGain, maxGain);
        }
    }
   
    return image.scaled(viewportWidth, viewportHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

