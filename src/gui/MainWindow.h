//
// Created by rika on 06/12/2019.
//

#ifndef SPEECH_ANALYSIS_MAINWINDOW_H
#define SPEECH_ANALYSIS_MAINWINDOW_H

#include "../audio/miniaudio.h"
#include <QtWidgets>
#include <QSharedPointer>
#include <utility>
#include "../audio/AudioDevices.h"
#include "../audio/AudioInterface.h"
#include "../audio/SineWave.h"
#include "../audio/NoiseFilter.h"
#include "AnalyserCanvas.h"
#include "PowerSpectrum.h"
#include "../analysis/Analyser.h"
#include "LPC/Frame/LPC_Frame.h"

using DevicePair = std::pair<bool, const ma_device_id *>;

Q_DECLARE_METATYPE(DevicePair);

constexpr int numFormants = 4;

extern QFont * appFont;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

protected:
#ifndef Q_OS_ANDROID
    bool eventFilter(QObject * obj, QEvent * event) override;
#endif

signals:
    void newFramesTracks(int nframe, double maxFreq, FormantMethod formantAlg, const std::deque<double> & pitches, const Formant::Frames & formants);
    void newFramesSpectrum(int nframe, int nNew, double maxFreq, std::deque<SpecFrame>::const_iterator begin, std::deque<SpecFrame>::const_iterator end);
    void newFramesLpc(double maxFreq, SpecFrame lpcFrame);
    void newFramesUI(int nframe, double maxFreq);

private:
    void loadSettings();

    void updateFields();
#ifndef Q_OS_ANDROID
    void updateDevices();
    void updateColorButtons();

    void toggleAnalyser();
    void toggleFullscreen();
#else
    void openSettings();
#endif

    ma_context maCtx;
    AudioDevices * devs;
    SineWave * sineWave;
    NoiseFilter * noiseFilter;
    AudioInterface * audioInterface;
    Analyser * analyser;

#ifdef Q_OS_MAC
    void * audioInterfaceMem;
#endif

    QTimer timer;

    QStringList fftSizes;

    QWidget * central;

#ifdef Q_OS_ANDROID
    QPushButton * inputSettings;
#else
    QDockWidget * fieldsDock;
    QDockWidget * settingsDock;

    QComboBox * inputDevIn;
    QPushButton * inputDevRefresh;
    QPushButton * inputDisplayDialog;
    QComboBox * inputFftSize;
    QSpinBox * inputLpOrder;
    QSpinBox * inputMaxFreq;
    QSpinBox * inputFrameLength;
    QSpinBox * inputFrameSpace;
    QDoubleSpinBox * inputWindowSpan;
    QComboBox * inputPitchAlg;
    QComboBox * inputFormantAlg;

    QWidget * dialogDisplay;
    QCheckBox * inputToggleSpectrum;
    QCheckBox * inputToggleTracks;
    QSpinBox * inputMinGain;
    QSpinBox * inputMaxGain;
    QComboBox * inputFreqScale;
    QSpinBox * inputPitchThick;
    QPushButton * inputPitchColor;
    QSpinBox * inputFormantThick;
    std::array<QPushButton *, numFormants> inputFormantColor;
    QComboBox * inputColorMap;

    QPushButton * inputPause;
    QPushButton * inputFullscreen;
#endif

    AnalyserCanvas * canvas;
    PowerSpectrum * powerSpectrum;

    QLineEdit * fieldPitch;
    std::vector<QLineEdit *> fieldFormant;
    QLineEdit * fieldOq;

};

#endif //SPEECH_ANALYSIS_MAINWINDOW_H
