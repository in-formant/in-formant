//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERCANVAS_H
#define SPEECH_ANALYSIS_ANALYSERCANVAS_H

#include <QtWidgets>
#include <QTimer>
#include <QSettings>
#include <mutex>
#include "rpmalloc.h"
#include "../Exceptions.h"
#include "../analysis/Analyser.h"
#include "../audio/SineWave.h"

#ifdef Q_OS_WASM
#   include "../qwasmsettings.h"
#endif

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

    void setFrequencyScale(int type, bool save = true);
    void setDrawSpectrum(bool draw, bool save = true);
    void setDrawTracks(bool draw, bool save = true);
    void setPitchColor(const QColor & color, bool save = true);
    void setFormantColor(int formantNb, const QColor & color, bool save = true);
    void setPitchThickness(int thick, bool save = true);
    void setFormantThickness(int thick, bool save = true);
    void setSpectrumColor(const QString & name, bool save = true);
    void setMinGainSpectrum(int gain, bool save = true);
    void setMaxGainSpectrum(int gain, bool save = true);

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

    double yFromFrequency(double frequency, double maxFreq);
    double frequencyFromY(int y, double maxFreq);

    void loadSettings() { QSettings s; loadSettings(s); }
    void saveSettings() { QSettings s; saveSettings(s); }

protected:
    void paintEvent(QPaintEvent * event) override;

public slots:
    void renderTracks(int nframe, double maxFreq, FormantMethod formantAlg, const rpm::deque<double> & pitches, const Formant::Frames & formants);
    void renderSpectrogram(int nframe, int nNew, double maxFreq, rpm::deque<SpecFrame>::const_iterator begin, rpm::deque<SpecFrame>::const_iterator end);
    void renderScaleAndCursor(int nframe, double maxFreq);

    void cursorMove(QObject * obj, bool toggle);
    void toggleSineWave(QObject * obj, bool toggle);

private:
    void loadSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

    void render();
    void renderPitchTrack(int nframe, double maxFreq, const rpm::deque<double> &pitches);
    void renderFormantTrack(int nframe, double maxFreq, FormantMethod formantAlg, const rpm::deque<double> &pitches, const Formant::Frames &formants);

    // Graphics-related members
    QPainter painter;

    std::mutex imageLock;
    QPixmap spectrogram;
    QPicture formantTracks;
    QPicture pitchTrack;
    QPicture scaleAndCursor;
    double upFactorTracks;
    double upFactorSpec;

    int nframe;
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
