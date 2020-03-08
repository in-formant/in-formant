//
// Created by rika on 06/12/2019.
//

#include "MainWindow.h"
#include "FFT/FFT.h"

MainWindow::MainWindow()
    : devs(), analyser(devs) {

    QPalette palette = this->palette();

    central = new QWidget;
    setCentralWidget(central);

    canvas = new AnalyserCanvas(analyser);

    vLayout1 = new QVBoxLayout(central);
    central->setLayout(vLayout1);
    {
        hLayout2 = new QHBoxLayout;
        vLayout1->addLayout(hLayout2);
        {
            vLayout6 = new QVBoxLayout;
            vLayout6->setSizeConstraint(QLayout::SetMaximumSize);
            vLayout6->setAlignment(Qt::AlignLeft);
            hLayout2->addLayout(vLayout6);
            {
                inputDevIn = new QComboBox;
                inputDevIn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

                inputDevRefresh = new QPushButton("Refresh list");
                inputDevRefresh->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

                connect(inputDevRefresh, &QPushButton::clicked,
                        [&]() { updateDevices(); });

                vLayout6->addWidget(inputDevIn, 0, Qt::AlignLeft);
                vLayout6->addWidget(inputDevRefresh, 0, Qt::AlignLeft);
            }

            hLayout5 = new QHBoxLayout;
            vLayout6->setSizeConstraint(QLayout::SetMaximumSize);
            vLayout6->setAlignment(Qt::AlignCenter);
            hLayout2->addLayout(hLayout5);
            {
                for (int i = 0; i < 4; ++i) {
                    auto field = new QLineEdit;

                    field->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                    field->setReadOnly(true);

                    fieldFormant.push_back(field);

                    hLayout5->addWidget(field, 0, Qt::AlignCenter);
                }
            }

            fieldPitch = new QLineEdit;

            fieldPitch->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            fieldPitch->setReadOnly(true);

            hLayout2->addWidget(fieldPitch, 0, Qt::AlignCenter);
        }

        vLayout1->addSpacing(16);

        hLayout3 = new QHBoxLayout;
        vLayout1->addLayout(hLayout3);
        {
            fLayout4 = new QFormLayout;
            hLayout3->addLayout(fLayout4);
            {
                inputToggleSpectrum = new QCheckBox;
                inputToggleSpectrum->setChecked(canvas->getDrawSpectrum());

                connect(inputToggleSpectrum, &QCheckBox::toggled,
                        [&](const bool checked) { canvas->setDrawSpectrum(checked); });

                inputFftSize = new QComboBox;
                inputFftSize->addItems({"256", "512", "1024", "2048", "4096", "8192", "16384"});
                inputFftSize->setCurrentIndex(1);

                connect(inputFftSize, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                        [&](const QString value) { analyser.setFftSize(value.toInt()); });

                inputMinGain = new QSpinBox;
                inputMinGain->setRange(-200, 60);
                inputMinGain->setSingleStep(10);
                inputMinGain->setSuffix(" dB");

                inputMaxGain = new QSpinBox;
                inputMaxGain->setRange(-200, 60);
                inputMaxGain->setSingleStep(10);
                inputMaxGain->setSuffix(" dB");
                
                connect(inputMinGain, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { canvas->setMinGainSpectrum(value);
                                               inputMaxGain->setMinimum(value + 10); });

                connect(inputMaxGain, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { canvas->setMaxGainSpectrum(value);
                                               inputMinGain->setMaximum(value - 10); });

                inputMinGain->setValue(canvas->getMinGainSpectrum());
                inputMaxGain->setValue(canvas->getMaxGainSpectrum());

                inputLpOrder = new QSpinBox;
                inputLpOrder->setRange(5, 22);
                inputLpOrder->setValue(analyser.getLinearPredictionOrder());

                connect(inputLpOrder, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser.setLinearPredictionOrder(value); });

                inputMaxFreq = new QSpinBox;
                inputMaxFreq->setRange(2500, 7000);
                //inputMaxFreq->setStepType(QSpinBox::AdaptiveDecimalStepType);
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

                for (int nb = 0; nb < 4; ++nb) {
                    auto input = new QPushButton;
                    
                    connect(input, &QPushButton::clicked,
                            [=] () {
                                QColor c = QColorDialog::getColor(
                                    canvas->getFormantColor(nb),
                                    this,
                                    QString("Select F%1 color").arg(nb + 1),
                                    QColorDialog::DontUseNativeDialog
                                );
                                if (c.isValid()) {
                                    canvas->setFormantColor(nb, c);
                                }
                            });

                    inputFormantColor[nb] = input;
                }

                fLayout4->addRow(tr("Overlay spectrogram:"), inputToggleSpectrum);
                fLayout4->addRow(tr("FFT size:"), inputFftSize);
                fLayout4->addRow(tr("Minimum gain:"), inputMinGain);
                fLayout4->addRow(tr("Maximum gain:"), inputMaxGain);
                fLayout4->addRow(tr("Linear prediction order:"), inputLpOrder);
                fLayout4->addRow(tr("Maximum frequency:"), inputMaxFreq);
                fLayout4->addRow(tr("Frequency scale:"), inputFreqScale);
                fLayout4->addRow(tr("Frame space:"), inputFrameSpace);
                fLayout4->addRow(tr("Analysis duration:"), inputWindowSpan);

                for (int nb = 0; nb < 4; ++nb) {
                    const QString labelStr = QString("F%1 color:").arg(nb + 1);
                    fLayout4->addRow(tr(qPrintable(labelStr)), inputFormantColor[nb]);
                }
            }

            hLayout3->addWidget(canvas);
            canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        }
    }

    setWindowTitle(WINDOW_TITLE);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    updateDevices();
    analyser.startThread();

    connect(&timer, &QTimer::timeout, [&]() {
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
    all_fft_cleanup();

}

void MainWindow::updateFields() {

    int frame = canvas->getSelectedFrame();

    const auto & formants = analyser.getFormantFrame(frame);
    double pitch = analyser.getPitchFrame(frame);

    QPalette palette = this->palette();

    if (pitch > 0) {
        fieldPitch->setText(QString("Fo = %1 Hz").arg(pitch, 0, 'f', 1));
        fieldPitch->setStyleSheet("color: cyan");
    } else {
        fieldPitch->setText("Unvoiced");
        fieldPitch->setStyleSheet(QString("color: %1").arg(palette.color(QPalette::Text).name()));
    }
    fieldPitch->adjustSize();

    for (int i = 0; i < 4; ++i) {
        if (i < formants.nFormants) {
            fieldFormant[i]->setText(QString("F%1 = %2 Hz").arg(i + 1).arg(round(formants.formant[i].frequency)));
        }
        else {
            fieldFormant[i]->setText("");
        }

        const QColor c = canvas->getFormantColor(i);
        inputFormantColor[i]->setStyleSheet(QString(" \
            QPushButton \
            { \
                background-color: %1; \
                border: 1px solid #32414B; \
                border-radius: 4px; \
                padding: 5px; \
                outline: none; \
                min-width: 80px; \
            } \
            QPushButton:hover \
            { \
                border: 1px solid #148CD2; \
            } \
        ").arg(c.name()));
    }

}

void MainWindow::updateDevices()
{
    const auto & inputs = devs.getInputs();
    const auto & outputs = devs.getOutputs();

    inputDevIn->disconnect();
    inputDevIn->clear();

    inputDevIn->addItem("", QVariant::fromValue(nullptr));

    for (const auto & dev : inputs) {
        const QString name = QString::fromLocal8Bit(dev.name.c_str());
        inputDevIn->addItem(name, QVariant::fromValue(&dev.id));
    }
    
    inputDevIn->setCurrentIndex(0);

    connect(inputDevIn, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&](const int index) {
                if (index >= 0) {
                    analyser.setInputDevice(inputDevIn->itemData(index).value<const ma_device_id *>());
                }
            });
}
