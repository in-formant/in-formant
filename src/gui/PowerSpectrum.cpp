#include <Eigen/Dense>
#include <iostream>
#include "PowerSpectrum.h"
#include "MFCC/MFCC.h"

PowerSpectrum::PowerSpectrum(Analyser * analyser, AnalyserCanvas * canvas)
    : spectrum(1, 1, QImage::Format_ARGB32_Premultiplied),
      lpcIm(1, 1, QImage::Format_ARGB32_Premultiplied),
      analyser(analyser), canvas(canvas)
{ 
    SpecFrame frame;
    frame.nfft = 1;
    frame.fs = 16000;
    frame.spec.setOnes(1);

    Eigen::ArrayXd one;
    one.setOnes(1);
    
    holdLength = 25;
    hold.resize(holdLength, frame);
    holdIndex = 0;
}

void PowerSpectrum::renderSpectrum(const int nframe, const int nNew, const double maximumFrequency, std::deque<SpecFrame>::const_iterator begin, std::deque<SpecFrame>::const_iterator end)
{
    using Eigen::ArrayXd;
  
    std::lock_guard<std::mutex> guard(imageLock);
   
    int minGain = canvas->getMinGainSpectrum();
    int maxGain = canvas->getMaxGainSpectrum();

    spectrum.fill(Qt::transparent);

    QPainter painter(&spectrum);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const double fs = begin->fs;
    
    // Advance the hold buffer.
    for (auto it = begin; it != end; ++it) {
        const auto & frame = *it;

        hold[holdIndex] = frame;
        holdIndex = (holdIndex + 1) % holdLength;
    }

    int maxNfft = 1;

    for (const auto & holdFrame : hold) {
        if (holdFrame.nfft > maxNfft) {
            maxNfft = holdFrame.nfft;
        }
    }
    
    ArrayXd maxHold(maxNfft);
    maxHold.setZero();

    double maxAmplitude = 1e-16;

    for (const auto & holdFrame : hold) {
        // Each frame needs to be resampled first.
        int gap = maxNfft / holdFrame.nfft;

        for (int i = 0; i < holdFrame.nfft; ++i) {
            for (int j = gap * i; j < gap * (i + 1); ++j) {

                if (holdFrame.spec(i) > maxHold(j)) {
                    maxHold(j) = holdFrame.spec(i);
                }

            }

            if (abs(holdFrame.spec(i)) > maxAmplitude) {
                maxAmplitude = abs(holdFrame.spec(i));
            }
        }
    }

    const double delta = fs / (2 * maxNfft);

    QPainterPath path;

    for (int i = 0; i < maxNfft; ++i) {
        double freq = i * delta;

        if (freq > maximumFrequency) {
            break;
        }

        double gain = std::clamp<double>(20 * std::log10(abs(maxHold(i))), minGain - 20, maxGain + 20);

        int x = targetWidth - (targetWidth * (maxGain - gain)) / (maxGain - minGain);
        int y = yFromFrequency(freq, maximumFrequency);
      
        if (i == 0) {
            path.moveTo(x, y);
        }
        else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(QColor(0xFFA500), 2));
    painter.drawPath(path);
}

void PowerSpectrum::renderLpc(double maximumFrequency, SpecFrame lpcSpectrum)
{
    std::lock_guard<std::mutex> guard(imageLock);
   
    const double fs = lpcSpectrum.fs;
    const int nfft = lpcSpectrum.nfft;
    const auto& spec = lpcSpectrum.spec;
    
    const double delta = fs / (2.0 * nfft);
   
    int minGain = canvas->getMinGainSpectrum();
    int maxGain = canvas->getMaxGainSpectrum();

    lpcIm.fill(Qt::transparent);

    QPainter painter(&lpcIm);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;

    for (int i = 0; i < nfft; ++i) {
        double freq = i * delta;

        if (freq > maximumFrequency) {
            break;
        }

        double gain = std::clamp<double>(20 * std::log10(abs(spec(i))), minGain - 20, maxGain + 20);

        int x = targetWidth - (targetWidth * (maxGain - gain)) / (maxGain - minGain);
        int y = yFromFrequency(freq, maximumFrequency);
      
        if (i == 0) {
            path.moveTo(x, y);
        }
        else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(QColor(0xC0C0C0), 2));
    painter.drawPath(path);
}

double PowerSpectrum::frequencyFromY(const int y, const double maximumFrequency)
{
    const double maxFreqMel = hz2mel(maximumFrequency);
    
    const double freqMel = ((targetHeight - y) * maxFreqMel) / (double) targetHeight;

    return mel2hz(freqMel);
}

int PowerSpectrum::yFromFrequency(const double freq, const double maximumFrequency)
{
    const double maxFreqMel = hz2mel(maximumFrequency);

    const double freqMel = hz2mel(freq);

    return targetHeight - (targetHeight * freqMel) / maxFreqMel;
}

void PowerSpectrum::paintEvent(QPaintEvent * event)
{
    targetWidth = width();
    targetHeight = height();

    if (spectrum.width() != targetWidth || spectrum.height() != targetHeight) {
        imageLock.lock();
        spectrum = QImage(targetWidth, targetHeight, QImage::Format_ARGB32_Premultiplied);
        spectrum.fill(Qt::transparent);
        imageLock.unlock();
    }

    if (lpcIm.width() != targetWidth || lpcIm.height() != targetHeight) {
        imageLock.lock();
        lpcIm = QImage(targetWidth, targetHeight, QImage::Format_ARGB32_Premultiplied);
        lpcIm.fill(Qt::transparent);
        imageLock.unlock();
    }

    painter.begin(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, targetWidth, targetHeight, Qt::black);

    imageLock.lock();
    painter.drawImage(0, 0, lpcIm);
    painter.drawImage(0, 0, spectrum);
    imageLock.unlock();
    
    painter.end();
}
