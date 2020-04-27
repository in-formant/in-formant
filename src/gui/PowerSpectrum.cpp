#include <Eigen/Dense>
#include <iostream>
#include "PowerSpectrum.h"
#include "MFCC/MFCC.h"

PowerSpectrum::PowerSpectrum(Analyser * analyser, AnalyserCanvas * canvas)
    : maxNfft(512),
      analyser(analyser), canvas(canvas)
{
    setObjectName("PowerSpectrum");

    holdLength = 25;

    rpm::vector<double> initialValues(holdLength);
    std::fill(initialValues.begin(), initialValues.end(), 1e-12);

    holdQues.resize(maxNfft, rpm::deque<double>(initialValues.begin(), initialValues.end()));
    holdSets.resize(maxNfft, rpm::multiset<double, std::greater<double>>(initialValues.begin(), initialValues.end()));
}

void PowerSpectrum::renderSpectrum(const int nframe, const int nNew, const double maximumFrequency, rpm::deque<SpecFrame>::const_iterator begin, rpm::deque<SpecFrame>::const_iterator end)
{
    using Eigen::ArrayXd;
  
    std::lock_guard<std::mutex> guard(imageLock);
   
    int minGain = canvas->getMinGainSpectrum();
    int maxGain = canvas->getMaxGainSpectrum();

    QPainter painter(&spectrum);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    
    const double fs = begin->fs;

    // Advance the hold buffer.
    for (auto it = begin; it != end; ++it) {
        advanceHold(*it);
    }

    const int nfft = maxNfft;

    rpm::vector<double> spec(nfft, 0.0);

    for (int i = 0; i < nfft; ++i) {
        spec[i] = *(holdSets[i].begin());
    }

    const double delta = fs / (2 * nfft);

    QPainterPath path;

    for (int i = 0; i < nfft; ++i) {
        double freq = i * delta;

        if (freq > maximumFrequency) {
            break;
        }

        double gain = std::clamp<double>(20 * std::log10(spec[i]), minGain - 20, maxGain + 20);

        int x = targetWidth - (targetWidth * (maxGain - gain)) / (maxGain - minGain);
        int y = canvas->yFromFrequency(freq, maximumFrequency);
      
        if (i == 0) {
            path.moveTo(x, y);
        }
        else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QColor(0xFFA500));
    painter.drawPath(path);
}

void PowerSpectrum::advanceHold(const SpecFrame& frame)
{
    // The frame needs to be resampled first.
    const int nfft = maxNfft;

    if (nfft >= frame.nfft) {
        int gap = nfft / frame.nfft;

        for (int i = 0; i < frame.nfft; ++i) {
            for (int j = gap * i; j < gap * (i + 1); ++j) {
                const double val = abs(frame.spec(i));

                const double back = holdQues[j].back();
                holdQues[j].pop_back();
                holdQues[j].push_front(val);

                holdSets[j].erase(holdSets[j].find(back));
                holdSets[j].emplace(val);
            }
        }
    }
    else {
        int gap = frame.nfft / nfft;

        for (int i = 0; i < nfft; ++i) {
            double val = 0;
            for (int j = gap * i; j < gap * (i + 1); ++j) {
                val += abs(frame.spec(j));
            }
            val /= (double) gap;
    
            const double back = holdQues[i].back();
            holdQues[i].pop_back();
            holdQues[i].push_front(val);

            holdSets[i].erase(holdSets[i].find(back));
            holdSets[i].emplace(val);
        }
    }
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

    QPainter painter(&lpcIm);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;

    for (int i = 0; i < nfft; ++i) {
        double freq = i * delta;

        if (freq > maximumFrequency) {
            break;
        }

        double gain = std::clamp<double>(20 * std::log10(abs(spec(i))) - 15, minGain - 20, maxGain + 20);

        int x = targetWidth - (targetWidth * (maxGain - gain)) / (maxGain - minGain);
        int y = canvas->yFromFrequency(freq, maximumFrequency);
      
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

void PowerSpectrum::paintEvent(QPaintEvent * event)
{
    targetWidth = width();
    targetHeight = height();

    painter.begin(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, targetWidth, targetHeight, Qt::black);

    imageLock.lock();
    painter.drawPicture(0, 0, lpcIm);
    painter.drawPicture(0, 0, spectrum);
    imageLock.unlock();

    const double maxFreq = analyser->getMaximumFrequency();
    const double freq = canvas->getSelectedFrequency();
    const int y = canvas->yFromFrequency(freq, maxFreq);

    painter.setPen(QPen(QColor(0x7F7F7F), 1));
    painter.drawLine(0, y, targetWidth, y);
    
    painter.end();
}
