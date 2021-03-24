#include "qpainterwrapper.h"
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
    int numSlices = slices.size();

    int iw = std::max(1024, 2 * numSlices);
    int ih = 600;
    int numBins = ih;

    Eigen::ArrayXXd amplitudes = Eigen::ArrayXXd::Zero(ih, iw);
    Eigen::ArrayXXd weights = Eigen::ArrayXXd::Zero(ih, iw);

    for (auto it = slices.begin(); it != slices.end(); ++it) {
        double time = it->first;
        const auto& coefs = it->second;

        Eigen::VectorXd mapped;

        // Check if we haven't already rendered this slice with these parameters.
        auto key = std::make_tuple(time, mFrequencyScale, mMinFrequency, mMaxFrequency);
        auto mapsIt = maps.find(key);
        if (mapsIt != maps.end()) {
            // We have.
            mapped = mapsIt->second;
        }
        else {
            // We haven't: render it now.
            auto& slice = coefs.magnitudes;
            auto& ytrans = constructTransformY(slice.rows(), numBins, mFrequencyScale, mMinFrequency, mMaxFrequency, FrequencyScale::Linear, 0, coefs.sampleRate / 2);
            mapped = (ytrans * slice).reverse();

            // Clear the map every now and then.
            if (maps.size() > mapsMaxSize) {
                maps.clear();
            }
            maps[key] = mapped;
        }

        int x1 = mapTimeToX(time - coefs.frameDuration, iw, mTimeStart, mTimeEnd);
        int x2 = mapTimeToX(time, iw, mTimeStart, mTimeEnd);
       
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

    struct pixel { uint8_t r, g, b, a; };
    
    rpm::vector<pixel> image(iw * ih);

    for (int iy = 0; iy < ih; ++iy) {
        for (int ix = 0; ix < iw; ++ix) {
            const double w = weights(iy, ix);
            if (w > 0) {
                const double amplitude = amplitudes(iy, ix) / w;
                const double adjusted = sqrt(amplitude / pow(10, mMaxGain / 20)) * 7;
                int index = std::clamp((int) std::floor(adjusted * 255), 0, 255);
                QColor c = QColor::fromRgb(cmap[index]);
                int r, g, b;
                c.getRgb(&r, &g, &b);
                image[iy * iw + ix].r = r;
                image[iy * iw + ix].g = g;
                image[iy * iw + ix].b = b;
                image[iy * iw + ix].a = 255;
            }
            else {
                image[iy * iw + ix].r = 0;
                image[iy * iw + ix].g = 0;
                image[iy * iw + ix].b = 0;
                image[iy * iw + ix].a = 255;
            }
        }
    }
  
    p->prepareSpectrogramDraw(); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    p->drawSpectrogram();
}

