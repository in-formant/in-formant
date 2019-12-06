//
// Created by rika on 06/12/2019.
//

#ifndef SPEECH_ANALYSIS_MAINWINDOW_H
#define SPEECH_ANALYSIS_MAINWINDOW_H

#include <QtWidgets>
#include <QSharedPointer>
#include "../canvas/AnalyserCanvas.h"

extern QFont appFont;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent * event);

private:
    Analyser analyser;

    QSharedPointer<QWidget> central;
    QSharedPointer<QVBoxLayout> vLayout1;
    QSharedPointer<QHBoxLayout> hLayout2;
    QSharedPointer<QHBoxLayout> hLayout3;
    QSharedPointer<QFormLayout> vLayout4;

    QSharedPointer<QSpinBox> inputLpOrder;
    QSharedPointer<QSpinBox> inputMaxFreq;

    QSharedPointer<AnalyserCanvas> canvas;

};

#endif //SPEECH_ANALYSIS_MAINWINDOW_H
