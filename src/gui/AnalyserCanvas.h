//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERCANVAS_H
#define SPEECH_ANALYSIS_ANALYSERCANVAS_H

#include <QtWidgets>
#include <QTimer>
#include <mutex>
#include "rpmalloc.h"
#include "../Exceptions.h"
#include "../analysis/Analyser.h"
#include "../audio/SineWave.h"

#define WINDOW_TITLE  "Speech analysis"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

class AnalyserCanvas : public QWidget
{
    Q_OBJECT
public:
    AnalyserCanvas(Analyser * analyser, SineWave * sineWave, NoiseFilter * noiseFilter) noexcept(false);
    ~AnalyserCanvas();

    void setSelectedFrame(int frame);
    [[nodiscard]] int getSelectedFrame() const;

    [[nodiscard]] double getSelectedFrequency() const;

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
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void paintEvent(QPaintEvent * event) override;


    friend class Analyser;
public slots:
    void renderTracks(int nframe, double maxFreq, FormantMethod formantAlg, const rpm::deque<double> & pitches, const Formant::Frames & formants);
    void renderSpectrogram(int nframe, int nNew, double maxFreq, rpm::deque<SpecFrame>::const_iterator begin, rpm::deque<SpecFrame>::const_iterator end);
    void renderScaleAndCursor(int nframe, double maxFreq);

private:
    void loadSettings();
    void saveSettings();

    void cursorMoveEvent(QMouseEvent * event);

    void render();
    void renderPitchTrack(int nframe, double maxFreq, const rpm::deque<double> &pitches);
    void renderFormantTrack(int nframe, double maxFreq, FormantMethod formantAlg, const rpm::deque<double> &pitches, const Formant::Frames &formants);

    double yFromFrequency(double frequency, double maxFreq);
    double frequencyFromY(int y, double maxFreq);

    // Graphics-related members
    QPainter painter;

    std::mutex imageLock;
    QImage spectrogram;
    QImage tracks;
    QImage scaleAndCursor;
    double upFactorTracks;
    double upFactorSpec;

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
    SineWave * sineWave;
    NoiseFilter * noiseFilter;

};


#endif //SPEECH_ANALYSIS_ANALYSERCANVAS_H
