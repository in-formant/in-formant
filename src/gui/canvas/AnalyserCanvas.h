//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERCANVAS_H
#define SPEECH_ANALYSIS_ANALYSERCANVAS_H

#include <QtWidgets>
#include <QTimer>
#include <mutex>
#include "../../Exceptions.h"
#include "../../analysis/Analyser.h"

#define WINDOW_TITLE  "Speech analysis"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

class AnalyserCanvas : public QWidget {
    Q_OBJECT
public:
    AnalyserCanvas(Analyser & analyser) noexcept(false);

    void setSelectedFrame(int frame);
    [[nodiscard]] int getSelectedFrame() const;

    void setFrequencyScale(int type);
    void setDrawSpectrum(bool draw);
    void setFormantColor(int formantNb, const QColor & color);
    void setMinGainSpectrum(int gain);
    void setMaxGainSpectrum(int gain);

    bool getDrawSpectrum() const;
    const QColor & getFormantColor(int formantNb) const;
    int getMinGainSpectrum() const;
    int getMaxGainSpectrum() const;

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void paintEvent(QPaintEvent * event) override;

private:
    void render();
    void renderFrame();
    void renderPitchTrack();
    void renderFormantTrack();
    void renderSpectrogram();
    void renderScaleAndCursor();

    double yFromFrequency(double frequency);
    double frequencyFromY(int y);

    // Graphics-related members
    QPainter painter;
    QTimer timer;
    std::mutex frameLock;

    std::array<QColor, 4> formantColors;
    int minGain, maxGain;

    QPixmap spectrogram;
    QPixmap tracks;

    int actualWidth, targetWidth, targetHeight;
    double maxFreq, maxFreqLog, maxFreqMel;

    // Rendering parameters
    bool drawSpectrum;
    int frequencyScaleType;
    int selectedFrame;
    double selectedFrequency;

    Analyser & analyser;

    friend class Analyser;

};


#endif //SPEECH_ANALYSIS_ANALYSERCANVAS_H
