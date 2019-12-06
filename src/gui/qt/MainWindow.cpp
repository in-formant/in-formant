//
// Created by rika on 06/12/2019.
//

#include "MainWindow.h"

MainWindow::MainWindow()
    : analyser() {

    QPalette palette = this->palette();

    central = new QWidget;
    setCentralWidget(central);

    vLayout1 = new QVBoxLayout(central);
    central->setLayout(vLayout1);
    {
        hLayout2 = new QHBoxLayout;
        vLayout1->addLayout(hLayout2);
        {
            hLayout5 = new QHBoxLayout;
            hLayout2->addLayout(hLayout5);
            {
                for (int i = 0; i < 4; ++i) {
                    auto field = new QLineEdit;
                    palette.setColor(QPalette::Text, QColor(formantColors[i]));
                    field->setPalette(palette);

                    field->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                    field->setReadOnly(true);

                    fieldFormant.push_back(field);

                    hLayout5->addWidget(field);
                }
            }

            fieldPitch = new QLineEdit;
            palette.setColor(QPalette::Text, Qt::cyan);
            fieldPitch->setPalette(palette);

            fieldPitch->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            fieldPitch->setReadOnly(true);

            hLayout2->addWidget(fieldPitch);
        }

        hLayout3 = new QHBoxLayout;
        vLayout1->addLayout(hLayout3);
        {
            fLayout4 = new QFormLayout;
            hLayout3->addLayout(fLayout4);
            fLayout4->setLabelAlignment(Qt::AlignLeft);
            {
                inputLpOrder = new QSpinBox;
                inputLpOrder->setRange(5, 22);
                inputLpOrder->setValue(analyser.getLinearPredictionOrder());

                connect(inputLpOrder, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser.setLinearPredictionOrder(value); });

                inputMaxFreq = new QSpinBox;
                inputMaxFreq->setRange(2500, 7000);
                inputMaxFreq->setStepType(QSpinBox::AdaptiveDecimalStepType);
                inputMaxFreq->setSuffix(" Hz");
                inputMaxFreq->setValue(analyser.getMaximumFrequency());

                connect(inputMaxFreq, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser.setMaximumFrequency(value); });

                inputFreqScale = new QComboBox;
                inputFreqScale->addItems({"Linear", "Logarithmic", "Mel"});
                inputFreqScale->setCurrentIndex(2);

                connect(inputFreqScale, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        [&](const int value) { canvas->setFrequencyScale(value); });

                inputFrameSpace = new QSpinBox;
                inputFrameSpace->setRange(5, 30);
                inputFrameSpace->setSingleStep(1);
                inputFrameSpace->setSuffix(" ms");
                inputFrameSpace->setValue(analyser.getFrameSpace().count());

                connect(inputFrameSpace, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser.setFrameSpace(std::chrono::milliseconds(value)); });

                inputWindowSpan = new QDoubleSpinBox;
                inputWindowSpan->setRange(2, 30);
                inputWindowSpan->setSingleStep(0.5);
                inputWindowSpan->setSuffix(" s");
                inputWindowSpan->setValue(analyser.getWindowSpan().count());

                connect(inputWindowSpan, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                        [&](const double value) { analyser.setWindowSpan(std::chrono::milliseconds(int(1000 * value))); });

                fLayout4->addRow(tr("Linear prediction order:"), inputLpOrder);
                fLayout4->addRow(tr("Maximum frequency:"), inputMaxFreq);
                fLayout4->addRow(tr("Frequency scale:"), inputFreqScale);
                fLayout4->addRow(tr("Frame space:"), inputFrameSpace);
                fLayout4->addRow(tr("Analysis duration:"), inputWindowSpan);
            }

            canvas = new AnalyserCanvas(analyser);
            hLayout3->addWidget(canvas);
            canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        }
    }

    setWindowTitle(WINDOW_TITLE);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    analyser.startThread();

    timer.callOnTimeout(this, [&]() {
        updateFields();
        canvas->repaint();
    });
    timer.start(1000.0 / 60.0);

    show();

}

MainWindow::~MainWindow() {
    delete central;
}

void MainWindow::closeEvent(QCloseEvent *event) {

    analyser.stopThread();

}

void MainWindow::updateFields() {

    QPalette palette = this->palette();

    int frame = canvas->getSelectedFrame();

    const auto & formants = analyser.getFormantFrame(frame);
    double pitch = analyser.getPitchFrame(frame);

    if (pitch > 0) {
        palette.setColor(QPalette::Text, Qt::cyan);
        fieldPitch->setText(QString("Fo = %1 Hz").arg(pitch, 0, 'f', 1));
    } else {
        fieldPitch->setText("Unvoiced");
    }
    fieldPitch->setPalette(palette);
    fieldPitch->adjustSize();

    for (int i = 0; i < 4; ++i) {
        if (i < formants.nFormants) {
            fieldFormant[i]->setText(QString("F%1 = %2 Hz").arg(i + 1).arg(round(formants.formant[i].frequency)));
        }
        else {
            fieldFormant[i]->setText("");
        }
    }

}