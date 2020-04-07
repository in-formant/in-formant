#include <Eigen/Dense>
#include <iostream>
#include "PowerSpectrum.h"
#include "MFCC/MFCC.h"

PowerSpectrum::PowerSpectrum(Analyser * analyser)
    : analyser(analyser)
{ 
    SpecFrame frame;
    frame.nfft = 1;
    frame.fs = 16000;
    frame.spec.setOnes(1);

    Eigen::ArrayXd one;
    one.setOnes(1);
    
    holdLength = 50;
    hold.resize(holdLength, frame);
    holdIndex = 0;

    minGain = -60;
    maxGain = 20;

    connect(&timer, &QTimer::timeout, [this]() {
        repaint();
    });
    timer.setTimerType(Qt::PreciseTimer);
    timer.start(1000.0 / 60.0);
}

PowerSpectrum::~PowerSpectrum()
{
}

void PowerSpectrum::renderSpectrum(const int nframe, const int nNew, const double maximumFrequency, std::deque<SpecFrame>::const_iterator begin, std::deque<SpecFrame>::const_iterator end)
{
    using Eigen::ArrayXd;

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

        double gain = std::clamp<double>(20 * std::log10(abs(maxHold(i))), minGain - 100, maxGain);

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

    painter.begin(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, targetWidth, targetHeight, QColor(0xC0C0C0));

    analyser->callIfNewFrames(
            1,
            [](auto&&... ts) {},
            [this](auto&&... ts) { renderSpectrum(std::forward<decltype(ts)>(ts)...); },
            [](auto&&... ts) {}
    );
    
    painter.end();
}
