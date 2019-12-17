//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AnalyserCanvas.h"
#include "MFCC/MFCC.h"
#include "../../Exceptions.h"

using namespace Eigen;

static constexpr int cmrCount = 9;
static const std::array<QColor, cmrCount> cmrMap = {
    QColor(0, 0, 0),
    QColor(38, 38, 128),
    QColor(77, 38, 191),
    QColor(153, 51, 128),
    QColor(255, 64, 38),
    QColor(230, 128, 0),
    QColor(230, 191, 26),
    QColor(230, 230, 128),
    QColor(255, 255, 255)
};

AnalyserCanvas::AnalyserCanvas(Analyser & analyser) noexcept(false)
    : selectedFrame(analyser.getFrameCount() - 1),
      maxFreq(0),
      drawSpectrum(true),
      frequencyScaleType(2),
      minGain(-60),
      maxGain(0),
      analyser(analyser)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    formantColors = { 
        0xFFA700,
        0xFF57D9,
        0x7FFF00,
        0x57C8C8,
    };

    analyser.setFrameCallback([&]() { renderFrame(); });
}

void AnalyserCanvas::render() { 

    std::lock_guard<std::mutex> guard(frameLock);
    
    const double scaleFactor = static_cast<double>(targetWidth) / static_cast<double>(actualWidth);

    painter.scale(scaleFactor, 1);

    if (drawSpectrum) {
        painter.drawPixmap(0, 0, spectrogram);
    }
    painter.drawPixmap(0, 0, tracks);

    painter.scale(1.0 / scaleFactor, 1);

    renderScaleAndCursor();

}

void AnalyserCanvas::renderFrame() {
    std::lock_guard<std::mutex> guard(frameLock);

    const int nframe = analyser.getFrameCount();
    const int xstep = actualWidth / nframe;
  
    QPainter scrollPainter;

    QPixmap tracksSnapshot = tracks.copy(0, 0, actualWidth, targetHeight);
    tracks.fill(Qt::transparent);
    scrollPainter.begin(&tracks); 
    scrollPainter.drawPixmap(QPoint(-xstep, 0), tracksSnapshot);
    scrollPainter.end();

    renderFormantTrack();
    renderPitchTrack();

    QPixmap spectrogramSnapshot = spectrogram.copy(0, 0, actualWidth, targetHeight);
    spectrogram.fill(Qt::transparent);
    scrollPainter.begin(&spectrogram);
    scrollPainter.drawPixmap(QPoint(-xstep, 0), spectrogramSnapshot);
    scrollPainter.end();

    renderSpectrogram();

}

void AnalyserCanvas::renderFormantTrack() {
    const int nframe = analyser.getFrameCount();
    const int iframe = nframe - 1;
    const int xstep = actualWidth / nframe;
    const int x = iframe * xstep;

    const auto & frame = analyser.getLastFormantFrame();
    const double pitch = analyser.getLastPitchFrame();

    QPainter tPainter(&tracks);

    int formantNb = 0;
    for (const auto & formant : frame.formant) {
        const int y = yFromFrequency(formant.frequency);

        QColor c;
        if (pitch > 0) {
            if (formantNb < 4) {
                c = formantColors[formantNb];
            } else {
                c = Qt::black;
            }
        }
        else {
            c = QColor(Qt::darkGray).darker(150);
        }

        tPainter.setPen(c);
        tPainter.setBrush(c);
        tPainter.drawRect(x - 2, y - 1, xstep + 4, 3); 
        
        formantNb++;
    }
}

void AnalyserCanvas::renderPitchTrack() {
    const int nframe = analyser.getFrameCount();
    const int iframe = nframe - 1;
    
    const int xstep = actualWidth / nframe;
    const int x = iframe * xstep;

    const double pitch = analyser.getLastPitchFrame();

    QPainter tPainter(&tracks);

    if (pitch > 0) {
        const double y = yFromFrequency(pitch);
        tPainter.setPen(Qt::cyan);
        tPainter.setBrush(Qt::cyan);
        tPainter.drawRect(x, y - 2, xstep + 1, 4);
    }
}

void AnalyserCanvas::renderScaleAndCursor() {
    constexpr int ruleSmall = 4;
    constexpr int ruleBig = 8;
    
    const double maximumFrequency = analyser.getMaximumFrequency();

    QFont font = painter.font();
    int oldSize = font.pixelSize();
    font.setPixelSize(13);
    painter.setFont(font);

    QFontMetrics metrics(font);

    for (double frequency = 0.0; frequency <= maximumFrequency; frequency += 100.0) {
        double y = yFromFrequency(frequency);

        if (fmod(frequency, 500.0) >= 1e-10) {
            painter.setPen(QColor(0xBBBBBB));
            painter.setBrush(QColor(0XBBBBBB));
            painter.drawRect(targetWidth - ruleSmall, y - 1, ruleSmall, 3);
        }
        else {
            painter.setPen(QColor(0xFFFFFF));
            painter.setBrush(QColor(0XFFFFFF));
            painter.drawRect(targetWidth - ruleBig, y - 1, ruleBig, 3);

            QSize sz = metrics.size(Qt::TextSingleLine, QString::number(frequency));
            if (y >= targetHeight) {
                y -= 2 + sz.height() / 4;
            }
            else if (y - sz.height() / 4 <= 0) {
                y += 2 + sz.height() / 4;
            }

            painter.setPen(0xFFFFFF);
            painter.drawText(targetWidth - 1 - ruleBig - sz.width(), y + sz.height() / 4, QString::number(frequency));
        }
    }

    const int nframe = analyser.getFrameCount();
    const int xstep = std::max(targetWidth / nframe, 1);
    const int x = (selectedFrame * xstep * targetWidth) / actualWidth;
    const int y = yFromFrequency(selectedFrequency);

    // Draw a vertical line where the selected frame is.
    painter.setPen(0x7F7F7F);
    painter.drawLine(x, 0, x, targetHeight);
    painter.drawLine(0, y, targetWidth, y);
    // Draw freq string right next to it.
    painter.setPen(0xFFFFFF);
    QString cursorFreqStr = QString("%1 Hz").arg(round(selectedFrequency));
    painter.drawText(targetWidth - ruleBig
                     - metrics.horizontalAdvance(QString::number(round(maximumFrequency)))
                     - 10
                     - metrics.horizontalAdvance(cursorFreqStr),
                     y - 10,
                     cursorFreqStr);

    font.setPixelSize(oldSize);
    painter.setFont(font);
}

void AnalyserCanvas::renderSpectrogram()
{
    struct Rectangle { int r, g, b; QRect rect; };
    const int nframe = analyser.getFrameCount();
    const int iframe = nframe - 1;
    
    const int xstep = std::max(targetWidth / nframe, 1);
    const int x = iframe * xstep;

    const SpecFrame & sframe = analyser.getLastSpectrumFrame();
    const double delta = sframe.fs / (2 * sframe.nfft);

    QVector<Rectangle> rects;
    rects.reserve(sframe.nfft * nframe);
    
    for (int i = 0; i < sframe.nfft; ++i) {
        const double y = yFromFrequency((i - 0.5) * delta);
        const double y2 = yFromFrequency((i + 0.5) * delta);

        if (y < 0 || y2 >= targetHeight)
            continue;

        double amplitude = abs(sframe.spec(i));
        double dB = std::clamp<double>(20.0 * log10(amplitude), minGain, maxGain);
       
        double cmrInd = (cmrCount - 1) - (cmrCount - 1) * static_cast<double>(maxGain - dB) / static_cast<double>(maxGain - minGain);
        
        int ileft = floor(cmrInd);
        int r, g, b;
        if (ileft < 0 || ileft >= cmrCount - 1) {
            cmrMap[ileft].getRgb(&r, &g, &b);
        }
        else {
            double a = cmrInd - ileft;
            int r1, r2, g1, g2, b1, b2;

            cmrMap[ileft].getRgb(&r1, &g1, &b1);
            cmrMap[ileft + 1].getRgb(&r2, &g2, &b2);

            r = (1 - a) * r1 + a * r2;
            g = (1 - a) * g1 + a * g2;
            b = (1 - a) * b1 + a * b2;
        }

        Rectangle rect = {
            r, g, b,
            QRect(x, y, xstep, y2 - y)
        };

        rects.push_back(std::move(rect));
    }

    QPainter sPainter(&spectrogram);

    for (const auto & rect : rects) {
        QColor c(rect.r, rect.g, rect.b);
        sPainter.setPen(c);
        sPainter.setBrush(c);
        sPainter.drawRect(rect.rect);
    }
}

void AnalyserCanvas::keyPressEvent(QKeyEvent * event) {
    const int key = event->key();

    if (key == Qt::Key_Escape) {
        window()->close();
    }
    else if (key == Qt::Key_P) {
        analyser.toggle();
    }
}

void AnalyserCanvas::mouseMoveEvent(QMouseEvent * event) {
    const auto p = event->localPos();

    const int nframe = analyser.getFrameCount();
    const double maximumFrequency = analyser.getMaximumFrequency();

    const int xstep = actualWidth / nframe;
    const double scaleFactor = static_cast<double>(actualWidth) / static_cast<double>(targetWidth);
    selectedFrame = (p.x() * scaleFactor) / xstep;
    //selectedFrame = std::clamp<int>(int(p.x() * scaleFactor) / xstep, 0, nframe - 1);
    selectedFrequency = std::clamp<double>(frequencyFromY(p.y()), 0.0, maximumFrequency);
}

double AnalyserCanvas::yFromFrequency(double frequency) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (maximumFrequency != maxFreq) {
        maxFreq = maximumFrequency;
        maxFreqLog = 1200.0 * std::log10(1.0 + maxFreq / 40.0);
        maxFreqMel = hz2mel(maxFreq);
    }

    if (frequencyScaleType == 0) {
        return (targetHeight * (maximumFrequency - frequency)) / maximumFrequency;
    }
    else if (frequencyScaleType == 1) {
        const double lf = 1200.0 * std::log10(1.0 + frequency / 40.0);

        return (targetHeight * (maxFreqLog - lf)) / maxFreqLog;
    }
    else if (frequencyScaleType == 2) {
        const double mel = hz2mel(frequency);

        return (targetHeight * (maxFreqMel - mel)) / maxFreqMel;
    }
    else {
        return 0;
    }
}

double AnalyserCanvas::frequencyFromY(int y) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (maximumFrequency != maxFreq) {
        maxFreq = maximumFrequency;
        maxFreqLog = 1200.0 * std::log10(1.0 + maxFreq / 40.0);
        maxFreqMel = hz2mel(maxFreq);
    }

    if (frequencyScaleType == 0) {
        return maximumFrequency - (y * maximumFrequency) / targetHeight;
    }
    else if (frequencyScaleType == 1) {
        const double lf = maxFreqLog - (y * maxFreqLog) / targetHeight;

        return 40.0 * pow(10.0, lf / 1200.0) - 1.0;
    }
    else if (frequencyScaleType == 2) {
        const double mel = maxFreqMel - (y * maxFreqMel) / targetHeight;

        return mel2hz(mel);
    }
    else {
        return 0;
    }
}

void AnalyserCanvas::paintEvent(QPaintEvent * event)
{
    targetWidth = width();
    targetHeight = height();

    const int nframe = analyser.getFrameCount();
    const int xstep = std::max(targetWidth / nframe, 1);
    actualWidth = nframe * xstep;

    if (spectrogram.width() != actualWidth || spectrogram.height() != targetHeight) {
        frameLock.lock();
        
        spectrogram = QPixmap(actualWidth, targetHeight);
        spectrogram.fill(Qt::black);
        
        tracks = QPixmap(actualWidth, targetHeight);
        tracks.fill(Qt::transparent);
        
        frameLock.unlock();
    }
    
    painter.begin(this);
    
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, targetWidth, targetHeight, Qt::black);
   
    render();

    painter.end();
}

void AnalyserCanvas::setSelectedFrame(int frame)
{
    selectedFrame = frame;
}

int AnalyserCanvas::getSelectedFrame() const
{
    return selectedFrame;
}

void AnalyserCanvas::setFrequencyScale(int type) {
    frequencyScaleType = type;
}

void AnalyserCanvas::setDrawSpectrum(bool toggle) {
    drawSpectrum = toggle;
}

bool AnalyserCanvas::getDrawSpectrum() const {
    return drawSpectrum;
}

void AnalyserCanvas::setFormantColor(int formantNb, const QColor & color) {
    formantColors[formantNb] = color;
}

const QColor & AnalyserCanvas::getFormantColor(int formantNb) const {
    return formantColors.at(formantNb);
}

void AnalyserCanvas::setMinGainSpectrum(int gain) {
    minGain = gain;
}

void AnalyserCanvas::setMaxGainSpectrum(int gain) {
    maxGain = gain;
}

int AnalyserCanvas::getMinGainSpectrum() const {
    return minGain;
}

int AnalyserCanvas::getMaxGainSpectrum() const {
    return maxGain;
}
