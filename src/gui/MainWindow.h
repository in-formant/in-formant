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
#include "AnalyserCanvas.h"
#include "PowerSpectrum.h"

using DevicePair = std::pair<bool, const ma_device_id *>;

Q_DECLARE_METATYPE(DevicePair);

extern QFont * appFont;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent * event) override;

private:
    void loadSettings();

    void updateDevices();
    void updateFields();
    void updateColorButtons();

    void toggleAnalyser();
    void toggleFullscreen();

    ma_context maCtx;
    AudioDevices * devs;
    Analyser * analyser;

    QTimer timer;

    QWidget * central;

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

    QPushButton * inputPause;
    QPushButton * inputFullscreen;

    QWidget * dialogDisplay;
    QCheckBox * inputToggleSpectrum;
    QCheckBox * inputToggleTracks;
    QSpinBox * inputMinGain;
    QSpinBox * inputMaxGain;
    QComboBox * inputFreqScale;
    QSpinBox * inputPitchThick;
    QPushButton * inputPitchColor;
    QSpinBox * inputFormantThick;
    std::array<QPushButton *, 4> inputFormantColor;
    QComboBox * inputColorMap;

    AnalyserCanvas * canvas;
    PowerSpectrum * powerSpectrum;

    QLineEdit * fieldPitch;
    std::vector<QLineEdit *> fieldFormant;

};

#endif //SPEECH_ANALYSIS_MAINWINDOW_H
