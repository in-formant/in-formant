//
// Created by rika on 06/12/2019.
//

#include "MainWindow.h"

MainWindow::MainWindow() {

    central.reset(new QWidget);
    setCentralWidget(central.get());

    layout.reset(new QVBoxLayout);
    central->setLayout(layout.get());

    analyser.startThread();

    canvas.reset(new AnalyserCanvas(analyser, this));
    canvas->startTimer();

    layout->addWidget(canvas.get());

    setWindowTitle(WINDOW_TITLE);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    show();

}
