//
// Created by rika on 06/12/2019.
//

#include "MainWindow.h"

MainWindow::MainWindow()
    : analyser() {

    central.reset(new QWidget);
    setCentralWidget(central.get());

    vLayout1.reset(new QVBoxLayout(central.get()));
    {
        hLayout2.reset(new QHBoxLayout);
        {
        }

        hLayout3.reset(new QHBoxLayout);
        {
            vLayout4.reset(new QFormLayout);
            {
                inputLpOrder.reset(new QSpinBox);
                inputLpOrder->setRange(5, 22);
                inputLpOrder->setValue(analyser.getLinearPredictionOrder());

                connect(inputLpOrder.get(), QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser.setLinearPredictionOrder(value); });

                inputMaxFreq.reset(new QSpinBox);
                inputMaxFreq->setRange(2500, 7000);
                inputMaxFreq->setStepType(QSpinBox::AdaptiveDecimalStepType);
                inputMaxFreq->setSuffix(" Hz");
                inputMaxFreq->setValue(analyser.getMaximumFrequency());

                connect(inputMaxFreq.get(), QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser.setMaximumFrequency(value); });

                vLayout4->addRow(tr("Linear prediction order:"), inputLpOrder.get());
                vLayout4->addRow(tr("&Maximum frequency:"), inputMaxFreq.get());
            }

            canvas.reset(new AnalyserCanvas(analyser, central.get()));
            canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            hLayout3->addLayout(vLayout4.get());
            hLayout3->addWidget(canvas.get());
        }

        vLayout1->addLayout(hLayout2.get());
        vLayout1->addLayout(hLayout3.get());
    }

    central->setLayout(vLayout1.get());

    setWindowTitle(WINDOW_TITLE);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    analyser.startThread();
    canvas->startTimer();

    show();

}

void MainWindow::closeEvent(QCloseEvent *event) {

    analyser.stopThread();

}