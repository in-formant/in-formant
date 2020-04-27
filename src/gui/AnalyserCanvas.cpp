//
// Created by clo on 12/09/2019.
//

#include <QTimer>
#include <iostream>
#include "AnalyserCanvas.h"
#include "ColorMaps.h"
#include "MFCC/MFCC.h"
#include "../Exceptions.h"
#include "../log/simpleQtLogger.h"

using namespace Eigen;

AnalyserCanvas::AnalyserCanvas(Analyser * analyser, SineWave * sineWave, NoiseFilter * noiseFilter) noexcept(false)
    : spectrogram(1, 1),
      upFactorTracks(1),
      upFactorSpec(1),
      maxFreq(0),
      minGain(-60),
      maxGain(0),
      drawSpectrum(true),
      drawTracks(true),
      frequencyScaleType(2),
      selectedFrame(analyser->getFrameCount() - 1),
      selectedFrequency(0),
      analyser(analyser),
      sineWave(sineWave),
      noiseFilter(noiseFilter)
{
    setObjectName("AnalyserCanvas");

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    spectrogram.fill(Qt::black);

    pitchColor = Qt::cyan;
    
    formantColors = { 
        0xFFA700,
        0xFF57D9,
        0x7FFF00,
        0x57C8C8,
    };

    loadSettings();
}

AnalyserCanvas::~AnalyserCanvas() {
    saveSettings();
}

void AnalyserCanvas::render() {

    imageLock.lock();

    if (drawSpectrum) {
        painter.drawPixmap(0, 0, spectrogram.scaled(targetWidth, targetHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    if (drawTracks) {
        painter.drawPicture(0, 0, formantTracks);
        painter.drawPicture(0, 0, pitchTrack);
    }

    painter.drawPicture(0, 0, scaleAndCursor);

    imageLock.unlock();

}

void AnalyserCanvas::renderTracks(const int nframe, const double maximumFrequency, FormantMethod formantAlg, const rpm::deque<double> &pitches, const Formant::Frames &formants) {
    std::lock_guard<std::mutex> guard(imageLock);

    renderFormantTrack(nframe, maximumFrequency, formantAlg, pitches, formants);
    renderPitchTrack(nframe, maximumFrequency, pitches);
}

void AnalyserCanvas::renderFormantTrack(const int nframe, const double maximumFrequency, FormantMethod formantAlg, const rpm::deque<double> &pitches, const Formant::Frames &formants) {

    const double xstep = upFactorTracks * (double) targetWidth / (double) nframe;

    QPainter tPainter(&formantTracks);

    tPainter.setPen(Qt::transparent);
    tPainter.setBrush(Qt::transparent);

    std::array<QPainterPath, 4> paths;
    std::array<bool, 4> startPath;

    for (bool& f : startPath) {
        f = true;
    }

    for (int iframe = 0; iframe < nframe; ++iframe) {
    
        const double x = iframe * xstep;

        const auto & frame = formants.at(iframe);
        const double pitch = pitches.at(iframe);

        int formantNb = 0;
        for (const auto & formant : frame.formant) {
            if (formant.frequency <= 0) {
                startPath[formantNb] = true;
                formantNb++;
                continue;
            }

            const double y = upFactorTracks * yFromFrequency(formant.frequency, maximumFrequency);

            QColor c;
            if (pitch != 0 && formantNb < 4) {    
                c = formantColors[formantNb];
            } else {
                c = Qt::black;
            }

            if (pitch == 0 || formantAlg == LP || formantNb >= 4) {
                tPainter.setPen(c);
                tPainter.setBrush(c);
                tPainter.drawEllipse(QPointF(x + xstep / 2, y - upFactorTracks / 2), formantThick / 2, formantThick / 2);
                if (formantAlg != LP && formantNb < 4) {
                    startPath[formantNb] = true;
                }
            }
            else {
                if (startPath[formantNb]) {
                    paths[formantNb].moveTo(QPointF{x, y});
                    startPath[formantNb] = false;
                }
                else {
                    paths[formantNb].lineTo(QPointF{x, y});
                }
            }
            
            formantNb++;
        }

        for (; formantNb < 4; ++formantNb) {
            startPath[formantNb] = true;
        }

    }

    for (int nb = 0; nb < signed(paths.size()); ++nb) {
        tPainter.setPen(QPen(formantColors[nb], upFactorTracks * formantThick, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        tPainter.setBrush(Qt::transparent);
        tPainter.drawPath(paths[nb]);
    }
}

void AnalyserCanvas::renderPitchTrack(const int nframe, const double maximumFrequency, const rpm::deque<double> &pitches) {
    const double xstep = upFactorTracks * (double) targetWidth / (double) nframe;
    
    QPainter tPainter(&pitchTrack);

    QPainterPath path;
    bool beginTrack = true;

    for (int iframe = 0; iframe < nframe; ++iframe) {

        const double x = iframe * xstep;

        const double pitch = pitches.at(iframe);

        if (pitch > 0) {
            const double y = upFactorTracks * yFromFrequency(pitch, maximumFrequency);
            
            if (beginTrack) {
                path.moveTo(QPointF{x, y});
                beginTrack = false;
            }
            else {
                path.lineTo(QPointF{x, y});
            }
        }
        else {
            beginTrack = true;
        }
    }
   
    tPainter.setBrush(Qt::transparent);
    tPainter.setPen(QPen(pitchColor, upFactorTracks * pitchThick, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    tPainter.drawPath(path);
}

void AnalyserCanvas::renderScaleAndCursor(const int nframe, const double maximumFrequency) {
    std::lock_guard<std::mutex> guard(imageLock);

    constexpr int ruleSmall = 4;
    constexpr int ruleBig = 8;

    QPainter painter(&scaleAndCursor);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QFont font = painter.font();
    int oldSize = font.pointSize();
    font.setPointSize(13);
    painter.setFont(font);

    QFontMetrics metrics(font);

    for (double frequency = 0.0; frequency <= maximumFrequency; frequency += 100.0) {
        double y = yFromFrequency(frequency, maximumFrequency);

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

    const double xstep = (double) targetWidth / (double) nframe;
    const int x = selectedFrame * xstep;
    const int y = yFromFrequency(selectedFrequency, maximumFrequency);

    // Draw a vertical line where the selected frame is.
    painter.setPen(QPen(QColor(0x7F7F7F), 2));
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

    font.setPointSize(oldSize);
    painter.setFont(font);
}

void AnalyserCanvas::renderSpectrogram(const int nframe, const int nNew, const double maximumFrequency, rpm::deque<SpecFrame>::const_iterator begin, rpm::deque<SpecFrame>::const_iterator end)
{
    std::lock_guard<std::mutex> guard(imageLock);
    
    struct Tile { int r, g, b; double y, y2; };

    const double xstep = upFactorSpec;

    QPainter scrollPainter(&spectrogram);
    scrollPainter.drawPixmap(QPointF{-nNew * xstep, 0}, spectrogram);
    scrollPainter.end();
   
    QPainter sPainter(&spectrogram);
    sPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const auto & cmrMap = colorMaps.find(colorMapName)->second;
    const int cmrCount = cmrMap.size();

    auto it = begin;

    int iframe = nframe - 1 - nNew;

    while (it != end) {
        rpm::vector<Tile> rects;

        const double x = iframe * xstep;
        const auto &sframe = *it;

        const double delta = sframe.fs / (2 * sframe.nfft);

        for (int i = 0; i < sframe.nfft; ++i) {
            const double y = upFactorSpec * yFromFrequency(i * delta, maximumFrequency);
            const double y2 = upFactorSpec * yFromFrequency((i + 1) * delta, maximumFrequency);

            if (y < 0 || y2 >= upFactorSpec * targetHeight)
                continue;

            double amplitude = abs(sframe.spec(i));
            if (!std::isnormal(amplitude)) {
                amplitude = 1e-16;
            }

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

            rects.push_back({ r, g, b, y, y2 });
        }
       
        Tile prevRect = {
            0, 0, 0
        };

        for (const auto & rect : rects) {
          
            // endy < starty
            double starty = rect.y;
            double endy = rect.y2;

            int r1 = prevRect.r;
            int g1 = prevRect.g;
            int b1 = prevRect.b;

            int r2 = rect.r;
            int g2 = rect.g;
            int b2 = rect.b;

            for (double y = endy; y < starty; y++) {
                double a = (double) (starty - y) / (double) (starty - endy);

                int r = (1 - a) * r1 + a * r2;
                int g = (1 - a) * g1 + a * g2;
                int b = (1 - a) * b1 + a * b2;

                sPainter.setPen(QPen(QColor(r, g, b), 1));
                sPainter.drawLine(QLineF{x, y, x + xstep - 1, y});
            }
            
            prevRect = rect;
        }

        iframe++;
        it++;
    }
}

double AnalyserCanvas::yFromFrequency(const double frequency, const double maximumFrequency) {
    if (maximumFrequency != maxFreq) {
        maxFreq = maximumFrequency;
        maxFreqLog = std::log(1.0 + maxFreq / 60.0);
        maxFreqMel = hz2mel(maxFreq);
    }

    if (frequencyScaleType == 0) {
        return (targetHeight * (maximumFrequency - frequency)) / maximumFrequency;
    }
    else if (frequencyScaleType == 1) {
        const double lf = std::log(1.0 + frequency / 60.0);

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

double AnalyserCanvas::frequencyFromY(const int y, const double maximumFrequency) {
    if (maximumFrequency != maxFreq) {
        maxFreq = maximumFrequency;
        maxFreqLog = std::log(1.0 + maxFreq / 60.0);
        maxFreqMel = hz2mel(maxFreq);
    }

    if (frequencyScaleType == 0) {
        return maximumFrequency - (y * maximumFrequency) / targetHeight;
    }
    else if (frequencyScaleType == 1) {
        const double lf = maxFreqLog - (y * maxFreqLog) / targetHeight;

        return 60.0 * (exp(lf) - 1.0);
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
  
    const int nframe = analyser->getFrameCount();
    const int specWidth = upFactorSpec * nframe;
    const int specHeight = upFactorSpec * targetHeight;

    if (spectrogram.width() != specWidth || spectrogram.height() != specHeight) {
        imageLock.lock();
        spectrogram = spectrogram.scaled(specWidth, specHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        imageLock.unlock();
    }

    painter.begin(this);
    
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, targetWidth, targetHeight, Qt::black);
   
    render();

    painter.end();
}

void AnalyserCanvas::cursorMove(QObject *obj, bool toggle)
{
    if (!toggle) return;

    const QString& name = obj->objectName();

    const auto p = this->mapFromGlobal(QCursor::pos());

    const double maximumFrequency = analyser->getMaximumFrequency();
    const double freq = std::clamp<double>(frequencyFromY(p.y(), maximumFrequency), 0.0, maximumFrequency);

    const int nframe = analyser->getFrameCount();
    const double xstep = (double) targetWidth / (double) nframe;
    const int frame = p.x() / xstep;

    if (name == "AnalyserCanvas") {
        selectedFrame = frame;
        selectedFrequency = freq;
    }
    else if (name == "PowerSpectrum") {
        selectedFrequency = freq;
    }
}

void AnalyserCanvas::toggleSineWave(QObject * obj, bool toggle) {
    if (obj == this && toggle) {
        const auto p = this->mapFromGlobal(QCursor::pos());
        const double maximumFrequency = analyser->getMaximumFrequency();
        const double freq = std::clamp<double>(frequencyFromY(p.y(), maximumFrequency), 0.0, maximumFrequency);

        sineWave->setFrequency(freq);
        sineWave->setPlaying(true);
    }
    else {
        sineWave->setPlaying(false);
    }
}

void AnalyserCanvas::setSelectedFrame(int frame)
{
    selectedFrame = frame;
}

int AnalyserCanvas::getSelectedFrame() const
{
    return selectedFrame;
}

double AnalyserCanvas::getSelectedFrequency() const
{
    return selectedFrequency;
}

void AnalyserCanvas::setFrequencyScale(int type, bool save) {
    frequencyScaleType = type;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int AnalyserCanvas::getFrequencyScale() const {
    return frequencyScaleType;
}

void AnalyserCanvas::setDrawSpectrum(bool toggle, bool save) {
    drawSpectrum = toggle;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

bool AnalyserCanvas::getDrawSpectrum() const {
    return drawSpectrum;
}

void AnalyserCanvas::setDrawTracks(bool toggle, bool save) {
    drawTracks = toggle;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

bool AnalyserCanvas::getDrawTracks() const {
    return drawTracks;
}

void AnalyserCanvas::setPitchColor(const QColor & color, bool save) {
    pitchColor = color;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

const QColor & AnalyserCanvas::getPitchColor() const {
    return pitchColor;
}

void AnalyserCanvas::setFormantColor(int formantNb, const QColor & color, bool save) {
    formantColors[formantNb] = color;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

const QColor & AnalyserCanvas::getFormantColor(int formantNb) const {
    return formantColors.at(formantNb);
}

void AnalyserCanvas::setPitchThickness(const int thick, bool save) {
    pitchThick = thick;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int AnalyserCanvas::getPitchThickness() const {
    return pitchThick;
}

void AnalyserCanvas::setFormantThickness(const int thick, bool save) {
    formantThick = thick;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int AnalyserCanvas::getFormantThickness() const {
    return formantThick;
}

void AnalyserCanvas::setSpectrumColor(const QString & name, bool save) {
    if (colorMaps.find(name) != colorMaps.end()) {
        colorMapName = name;
    }

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

const QString & AnalyserCanvas::getSpectrumColor() const {
    return colorMapName;
}

void AnalyserCanvas::setMinGainSpectrum(int gain, bool save) {
    minGain = gain;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

void AnalyserCanvas::setMaxGainSpectrum(int gain, bool save) {
    maxGain = gain;

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int AnalyserCanvas::getMinGainSpectrum() const {
    return minGain;
}

int AnalyserCanvas::getMaxGainSpectrum() const {
    return maxGain;
}

void AnalyserCanvas::loadSettings(QSettings& settings) {
    L_INFO("Loading canvas settings...");

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { loadSettings(settings); });
    }
    else {
        settings.beginGroup("canvas");

        setFrequencyScale(settings.value("freqScale", 2).value<int>(), false);
        setDrawSpectrum(settings.value("drawSpectrum", true).value<bool>(), false);
        setDrawTracks(settings.value("drawTracks", true).value<bool>(), false);
      
        setPitchColor(settings.value("pitchColor", pitchColor).value<QColor>(), false);

        for (int i = 0; i < 4; ++i) {
            setFormantColor(i, settings.value(QString("formantColor/%1").arg(i), formantColors[i]).value<QColor>(), false);
        }

#ifdef Q_OS_ANDROID
        setPitchThickness(settings.value("pitchThickness", 6).value<int>(), false);
        setFormantThickness(settings.value("formantThickness", 6).value<int>(), false);
#else
        setPitchThickness(settings.value("pitchThickness", 4).value<int>(), false);
        setFormantThickness(settings.value("formantThickness", 4).value<int>(), false);
#endif

        setSpectrumColor(settings.value("spectrumColorMap", "iZotope").value<QString>(), false);

        setMinGainSpectrum(settings.value("minGain", -40).value<int>(), false);
        setMaxGainSpectrum(settings.value("maxGain", 20).value<int>(), false);

        settings.endGroup();
    }
}

void AnalyserCanvas::saveSettings(QSettings& settings) {
    L_INFO("Saving canvas settings..."); 

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { saveSettings(settings); });
    }
    else {
        settings.beginGroup("canvas");

        settings.setValue("freqScale", frequencyScaleType);
        settings.setValue("drawSpectrum", drawSpectrum);
        settings.setValue("drawTracks", drawTracks);

        settings.setValue("pitchColor", pitchColor);

        for (int i = 0; i < 4; ++i) {
            settings.setValue(QString("formantColor/%1").arg(i), formantColors[i]);
        }

        settings.setValue("pitchThickness", pitchThick);
        settings.setValue("formantThickness", formantThick);

        settings.setValue("spectrumColorMap", colorMapName);

        settings.setValue("minGain", minGain);
        settings.setValue("maxGain", maxGain);

        settings.endGroup();
    }
}
