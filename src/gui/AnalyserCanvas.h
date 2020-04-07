//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERCANVAS_H
#define SPEECH_ANALYSIS_ANALYSERCANVAS_H

#include <QtWidgets>
#include <QTimer>
#include <mutex>
#include "../Exceptions.h"
#include "../analysis/Analyser.h"

#define WINDOW_TITLE  "Speech analysis"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

class AnalyserCanvas : public QWidget {
    Q_OBJECT
public:
    AnalyserCanvas(Analyser * analyser) noexcept(false);
    ~AnalyserCanvas();

    void setSelectedFrame(int frame);
    [[nodiscard]] int getSelectedFrame() const;

    void setFrequencyScale(int type);
    void setDrawSpectrum(bool draw);
    void setDrawTracks(bool draw);
    void setPitchColor(const QColor & color);
    void setFormantColor(int formantNb, const QColor & color);
    void setPitchThickness(int thick);
    void setFormantThickness(int thick);
    void setSpectrumColor(const QString & name);
    void setMinGainSpectrum(int gain);
    void setMaxGainSpectrum(int gain);

    int getFrequencyScale() const;
    bool getDrawSpectrum() const;
    bool getDrawTracks() const;
    const QColor & getPitchColor() const;
    const QColor & getFormantColor(int formantNb) const;
    int getPitchThickness() const;
    int getFormantThickness() const;
    const QString & getSpectrumColor() const;
    int getMinGainSpectrum() const;
    int getMaxGainSpectrum() const;

protected:
    void mouseMoveEvent(QMouseEvent * event) override;
    void paintEvent(QPaintEvent * event) override;

private:
    void loadSettings();
    void saveSettings();

    void render();
    void renderTracks(int nframe, double maxFreq, const std::deque<double> &pitches, const Formant::Frames &formants);
    void renderPitchTrack(int nframe, double maxFreq, const std::deque<double> &pitches);
    void renderFormantTrack(int nframe, double maxFreq, const std::deque<double> &pitches, const Formant::Frames &formants);
    void renderSpectrogram(int nframe, int nNew, double maxFreq, std::deque<SpecFrame>::const_iterator begin, std::deque<SpecFrame>::const_iterator end);
    void renderScaleAndCursor(int nframe, double maxFreq);

    double yFromFrequency(double frequency, double maxFreq);
    double frequencyFromY(int y, double maxFreq);

    // Graphics-related members
    QPainter painter;
    QTimer timer;
    std::mutex frameLock;

    QImage spectrogram;
    QImage tracks;
    double upFactor;

    int actualWidth, targetWidth, targetHeight;
    double maxFreq, maxFreqLog, maxFreqMel;

    // Rendering parameters
    QColor pitchColor;
    std::array<QColor, 4> formantColors;
    int pitchThick;
    int formantThick;
    QString colorMapName;

    int minGain, maxGain;

    bool drawSpectrum, drawTracks;
    int frequencyScaleType;
    int selectedFrame;
    double selectedFrequency;

    Analyser * analyser;

    friend class Analyser;

};


#endif //SPEECH_ANALYSIS_ANALYSERCANVAS_H
