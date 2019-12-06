//
// Created by rika on 06/12/2019.
//

#ifndef SPEECH_ANALYSIS_MAINWINDOW_H
#define SPEECH_ANALYSIS_MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QWidget>
#include <QSharedPointer>
#include "../canvas/AnalyserCanvas.h"

class MainWindow : public QMainWindow {
public:
    MainWindow();

private:
    Analyser analyser;

    QSharedPointer<QWidget> central;
    QSharedPointer<QVBoxLayout> layout;
    QSharedPointer<AnalyserCanvas> canvas;

};

#endif //SPEECH_ANALYSIS_MAINWINDOW_H
