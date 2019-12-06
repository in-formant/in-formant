//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERCANVAS_H
#define SPEECH_ANALYSIS_ANALYSERCANVAS_H

#include <QSharedPointer>
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <vector>
#include "../../Exceptions.h"
#include "../../analysis/Analyser.h"

#define WINDOW_TITLE  "Speech analysis"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define WINDOW_FONT     "fonts/open-sans/OpenSans-Regular.ttf"
#define WINDOW_FONTSIZE 24

class AnalyserCanvas : public QWidget {
    Q_OBJECT
public:
    AnalyserCanvas(Analyser & analyser, QWidget * parent) noexcept(false);

    void startTimer();

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void paintEvent(QPaintEvent * event) override;

private:
    void mainLoop();

    void render();
    void renderGraph();

    double yFromFrequency(double frequency);
    double frequencyFromY(int y);

    // Graphics-related members
    QPainter painter;
    QTimer timer;
    std::thread renderThread;

    int targetWidth, targetHeight;

    // Rendering parameters
    bool renderRaw, renderLogScale;
    int selectedFrame;
    double selectedFrequency;

    Analyser & analyser;

    friend class Analyser;

};


#endif //SPEECH_ANALYSIS_ANALYSERCANVAS_H
