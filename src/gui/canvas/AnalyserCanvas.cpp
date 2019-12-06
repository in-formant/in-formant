//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AnalyserCanvas.h"
#include "../../lib/MFCC/MFCC.h"
#include "../../Exceptions.h"

using namespace Eigen;

AnalyserCanvas::AnalyserCanvas(Analyser & analyser) noexcept(false)
    : selectedFrame(analyser.getFrameCount() - 1),
      frequencyScaleType(2),
      analyser(analyser)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void AnalyserCanvas::render() {
    const auto & metrics = painter.fontMetrics();

    const int nframe = analyser.getFrameCount();
    const int xstep = std::max(nframe / targetWidth, 1);

    const double maximumFrequency = analyser.getMaximumFrequency();

    const int formantRadius = 2;
    const int ruleSmall = 4;
    const int ruleBig = 8;

    int x, y;

    for (int iframe = 0; iframe < nframe; ++iframe) {
        const auto & frame = analyser.getFormantFrame(iframe);
        double pitch = analyser.getPitchFrame(iframe);

        x = (iframe * targetWidth) / nframe;

        int formantNb = 0;
        for (const auto & formant : frame.formant) {
            y = yFromFrequency(formant.frequency);

            if (pitch > 0) {
                painter.setPen(QColor(formantColors[formantNb]));
                painter.setBrush(QColor(formantColors[formantNb]));
                painter.drawEllipse(QPoint{x, y}, formantRadius, formantRadius);
            } else {
                painter.fillRect(x - 1, y - 1, 3, 3, Qt::red);
            }

            formantNb++;
        }

        if (pitch > 0) {
            y = yFromFrequency(pitch);
            painter.fillRect(x - 1, y - 1, 3, 3, Qt::cyan);
        }

        x += xstep;
    }

    QFont font = painter.font();
    int oldSize = font.pixelSize();
    font.setPixelSize(13);
    painter.setFont(font);

    for (double frequency = 0.0; frequency <= maximumFrequency; frequency += 100.0) {
        y = yFromFrequency(frequency);

        if (fmod(frequency, 500.0) >= 1e-10) {
            painter.fillRect(targetWidth - ruleSmall, y - 1, ruleSmall, 3, 0xBBBBBB7F);
        }
        else {
            painter.fillRect(targetWidth - ruleBig, y - 1, ruleBig, 3, 0xBBBBBB);

            QSize sz = metrics.size(Qt::TextSingleLine, QString::number(frequency));
            if (y >= targetHeight) {
                y -= 2 + sz.height() / 4;
            }
            else if (y - sz.height() / 4 <= 0) {
                y += 2 + sz.height() / 4;
            }

            painter.setPen(0xBBBBBB);
            painter.drawText(x - ruleBig - sz.width(), y + sz.height() / 4, QString::number(frequency));
        }
    }

    x = (selectedFrame * targetWidth) / nframe;
    y = yFromFrequency(selectedFrequency);

    // Draw a vertical line where the selected frame is.
    painter.setPen(0x7F7F7F7F);
    painter.drawLine(x, 0, x, targetHeight);
    painter.drawLine(0, y, targetWidth, y);
    // Draw freq string right next to it.
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
    const auto p = event->pos();

    const int nframe = analyser.getFrameCount();
    const double maximumFrequency = analyser.getMaximumFrequency();

    selectedFrame = std::clamp<int>((p.x() * nframe) / targetWidth, 0, nframe - 1);
    selectedFrequency = std::clamp<double>(frequencyFromY(p.y()), 0.0, maximumFrequency);
}

double AnalyserCanvas::yFromFrequency(double frequency) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (frequencyScaleType == 0) {
        return (targetHeight * (maximumFrequency - frequency)) / maximumFrequency;
    }
    else if (frequencyScaleType == 1) {
        const double maxLf = 1200.0 * std::log10(1.0 + maximumFrequency / 40.0);
        const double lf = 1200.0 * std::log10(1.0 + frequency / 40.0);

        return (targetHeight * (maxLf - lf)) / maxLf;
    }
    else if (frequencyScaleType == 2) {
        const double maxMel = hz2mel(maximumFrequency);
        const double mel = hz2mel(frequency);

        return (targetHeight * (maxMel - mel)) / maxMel;
    }
    else {
        return 0;
    }
}

double AnalyserCanvas::frequencyFromY(int y) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (frequencyScaleType == 0) {
        return maximumFrequency - (y * maximumFrequency) / targetHeight;
    }
    else if (frequencyScaleType == 1) {
        const double maxLf = 1200.0 * std::log10(1.0 + maximumFrequency / 40.0);
        const double lf = maxLf - (y * maxLf) / targetHeight;

        return 40.0 * pow(10.0, lf / 1200.0) - 1.0;
    }
    else if (frequencyScaleType == 2) {
        const double maxMel = hz2mel(maximumFrequency);
        const double mel = maxMel - (y * maxMel) / targetHeight;

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

    painter.begin(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.fillRect(0, 0, targetWidth, targetHeight, Qt::black);
    render();
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, targetWidth, targetHeight);
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