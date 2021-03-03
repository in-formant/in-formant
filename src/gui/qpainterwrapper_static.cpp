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

QImage QPainterWrapper::drawSpectrogram(
            const rpm::vector<std::pair<double, Main::SpectrogramCoefs>>& slices,
            double timeStart,
            double timeEnd,
            FrequencyScale frequencyScale,
            double minFrequency,
            double maxFrequency,
            double maxGain)
{
    int numSlices = slices.size();

    int iw = 1024;
    int ih = 1024;
    int numBins = ih;

    Eigen::ArrayXXd amplitudes = Eigen::ArrayXXd::Zero(ih, iw);
    Eigen::ArrayXXd weights = Eigen::ArrayXXd::Zero(ih, iw);

    for (auto it = slices.begin(); it != slices.end(); ++it) {
        double time = it->first;
        const auto& coefs = it->second;

        auto& slice = coefs.magnitudes;

        auto ytrans = constructTransformY(slice.rows(), numBins, frequencyScale, minFrequency, maxFrequency, FrequencyScale::Linear, 0, coefs.sampleRate / 2);
          
        Eigen::VectorXd mapped = (ytrans * slice).reverse();

        int x1 = mapTimeToX(time - coefs.frameDuration, iw, timeStart, timeEnd);
        int x2 = mapTimeToX(time, iw, timeStart, timeEnd);
       
        auto window = Analysis::blackmanHarrisWindow(x2 - x1 + 1);

        for (int iy = 0; iy < mapped.size(); ++iy) {
            for (int ix = x1; ix <= x2; ++ix) {
                if (ix >= 0 && ix < iw) {
                    amplitudes(iy, ix) += window[ix - x1] * mapped(iy);
                    weights(iy, ix) += window[ix - x1];
                }
            }
        }
    }

    QImage image(iw, ih, QImage::Format_Indexed8);
    image.setColorTable(cmap);

    for (int iy = 0; iy < ih; ++iy) {
        uint8_t *scanLineBits = reinterpret_cast<uint8_t *>(image.scanLine(iy));
        for (int ix = 0; ix < iw; ++ix) {
            const double w = weights(iy, ix);
            if (w > 0) {
                const double amplitude = amplitudes(iy, ix) / w;
                const double adjusted = sqrt(amplitude / pow(10, maxGain / 20)) * 7;
                int index = std::clamp((int) std::floor(adjusted * 255), 0, 255);
                scanLineBits[ix] = index;
            }
            else {
                scanLineBits[ix] = 0;
            }
        }
    }
    
    return image;
}

