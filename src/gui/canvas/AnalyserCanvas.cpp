//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AnalyserCanvas.h"
#include "../../lib/MFCC/MFCC.h"
#include "../../Exceptions.h"

using namespace Eigen;

static constexpr QRgb formantColors[9] = {
    0xFFA700,
    0xFF57D9,
    0x7FFF00,
    0x57C8C8,
    0xC8A7FF,
    0x00A79C,
    0xFFFFFF, // unused
    0xFFFFFF,
    0xFFFFFF,
};

AnalyserCanvas::AnalyserCanvas(Analyser & analyser, QWidget * parent) noexcept(false)
    : QWidget(parent),
      targetWidth(WINDOW_WIDTH),
      targetHeight(WINDOW_HEIGHT),
      selectedFrame(analysisFrameCount - 1),
      renderRaw(false),
      renderLogScale(true),
      analyser(analyser)
{
    setMouseTracking(true);
}

void AnalyserCanvas::render() {
#define ScaledPos(x, y) QPointF{targetWidth * (x), targetHeight * (y)}

    QPointF pos;

    //--- BACKGROUND
    renderGraph();

    //--- FOREGROUND

    const auto & frame = analyser.getFormantFrame(selectedFrame, renderRaw);
    double pitch = analyser.getPitchFrame(selectedFrame);

    // Draw formant estimation strings.
    for (int i = 0; i < frame.nFormants; ++i) {
        const auto & Fi = frame.formant.at(i);

        painter.setPen(formantColors[i]);

        pos = ScaledPos(0.01, 0.02 + i * 0.04);
        painter.drawText(pos, QString("F%1 = %2 Hz")
                                    .arg(i + 1)
                                    .arg(Fi.frequency, 0, 'f', 2));
    }

    if (pitch > 0) {
        painter.setPen(Qt::white);
        pos = ScaledPos(0.8, 0.02);
        painter.drawText(pos, QString("Voiced: %1 Hz").arg(round(pitch)));
    }

    painter.setPen(QRgb(0xBEBEBE));

    pos = ScaledPos(0.15, 0.02);
    painter.drawText(pos, QString("LP order: %1").arg(analyser.getLinearPredictionOrder()));

    pos = ScaledPos(0.15, 0.06);
    painter.drawText(pos, QString("Max freq.: %1").arg(round(analyser.getMaximumFrequency())));
}

void AnalyserCanvas::keyPressEvent(QKeyEvent * event) {
    const int key = event->key();

    if (key == Qt::Key_P) {
        analyser.toggle();
    }
    if (key == Qt::Key_S) {
        renderLogScale = !renderLogScale;
    }

    int parIncr = (key == Qt::Key_Up ? 1
                    : (key == Qt::Key_Down ? -1 : 0));
    if (parIncr != 0) {
        // LPC Order
        /*if (key == Qt::Key::L) {
            int val = analyser.getLinearPredictionOrder();
            analyser.setLinearPredictionOrder(val + parIncr);
        }
        else if (key == Qt::Key::P) {*/
            double val = analyser.getMaximumFrequency();
            analyser.setMaximumFrequency(val + 100.0 * parIncr);
        //}
    }
}

void AnalyserCanvas::mouseMoveEvent(QMouseEvent * event) {
    const auto p = event->pos();

    constexpr int nframe = analysisFrameCount;
    const double maximumFrequency = analyser.getMaximumFrequency();

    selectedFrame = std::clamp<int>((p.x() * nframe) / targetWidth, 0, nframe - 1);
    selectedFrequency = std::clamp<double>(frequencyFromY(p.y()), 0.0, maximumFrequency);
}

double AnalyserCanvas::yFromFrequency(double frequency) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (renderLogScale) {
        const double maxMel = hz2mel(maximumFrequency);
        const double mel = hz2mel(frequency);

        return (targetHeight * (maxMel - mel)) / maxMel;
    }
    else {
        return (targetHeight * (maximumFrequency - frequency)) / maximumFrequency;
    }
}

double AnalyserCanvas::frequencyFromY(int y) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (renderLogScale) {
        const double maxMel = hz2mel(maximumFrequency);
        const double mel = maxMel - (y * maxMel) / targetHeight;

        return mel2hz(mel);
    }
    else {
        return maximumFrequency - (y * maximumFrequency) / targetHeight;
    }
}

void AnalyserCanvas::renderGraph() {
    const QFontMetrics metrics(painter.font());

    constexpr int nframe = analysisFrameCount;
    const int xstep = std::max(nframe / targetWidth, 1);

    const double maximumFrequency = analyser.getMaximumFrequency();

    const int formantRadius = 2;
    const int ruleSmall = 4;
    const int ruleBig = 8;

    int x, y;

    for (int iframe = 0; iframe < nframe; ++iframe) {
        const auto & frame = analyser.getFormantFrame(iframe, renderRaw);
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
                painter.fillRect(QRect{x - 1, y - 1, 3, 3}, Qt::red);
            }

            formantNb++;
        }

        if (pitch > 0) {
            y = yFromFrequency(pitch);
            painter.fillRect(QRect{x - 1, y - 1, 3, 3}, Qt::cyan);
        }

        x += xstep;
    }

    for (double frequency = 0.0; frequency <= maximumFrequency; frequency += 100.0) {
        y = yFromFrequency(frequency);

        if (fmod(frequency, 500.0) >= 1e-10) {
            painter.fillRect(QRect{targetWidth - ruleSmall, y - 1, ruleSmall, 3}, 0xBBBBBB7F);
        }
        else {
            painter.fillRect(QRect{targetWidth - ruleBig, y - 1, ruleBig, 3}, 0xBBBBBB);

            QSize sz = metrics.size(Qt::TextSingleLine, QString::number(frequency));
            if (y >= targetHeight) {
                y -= sz.height() / 4;
            }
            else if (y - sz.height() / 4 <= 0) {
                y += sz.height() / 4;
            }

            painter.setPen(0xBBBBBB);
            painter.setLayoutDirection(Qt::RightToLeft);
            painter.drawText(QPoint{x - ruleBig - 4 - sz.width(), y + sz.height() / 4}, QString::number(frequency));
            painter.setLayoutDirection(Qt::LayoutDirectionAuto);
        }
    }

    x = (selectedFrame * targetWidth) / nframe;
    y = yFromFrequency(selectedFrequency);

    // Draw a vertical line where the selected frame is.
    painter.setPen(0x7F7F7F7F);
    painter.drawLine(QLine{x, 0, x, targetHeight});
    painter.drawLine(QLine{0, y, targetWidth, y});
    // Draw freq string right next to it.
    QString cursorFreqStr = QString("%1 Hz").arg(round(selectedFrequency));
    painter.drawText(QPoint{targetWidth - ruleBig
                             - metrics.horizontalAdvance(QString::number(maximumFrequency))
                             - 10
                             - metrics.horizontalAdvance(cursorFreqStr),
                            y - 10},
                     cursorFreqStr);
}

void AnalyserCanvas::startTimer()
{
    timer.callOnTimeout(this, [&]() { repaint(); });
    timer.start(1000.0 / 60.0);
}

void AnalyserCanvas::paintEvent(QPaintEvent * event)
{
    targetWidth = width();
    targetHeight = height();

    painter.begin(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.fillRect(0, 0, width(), height(), Qt::black);
    render();
    painter.end();
}