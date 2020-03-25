//
// Created by rika on 06/12/2019.
//

#include "MainWindow.h"
#include "FFT/FFT.h"
#include "../log/simpleQtLogger.h"

constexpr int maxWidthComboBox = 200;

MainWindow::MainWindow() {

    L_INFO("Initialising miniaudio context...");

    std::vector<ma_backend> backends{
        ma_backend_wasapi,
        ma_backend_winmm,
        ma_backend_dsound,

        ma_backend_coreaudio,

        ma_backend_pulseaudio,
        ma_backend_jack,
        ma_backend_alsa,
    
        ma_backend_sndio,
        ma_backend_audio4,
        ma_backend_oss,

        ma_backend_aaudio,
        ma_backend_opensl,

        ma_backend_webaudio,

        ma_backend_null
    };

    ma_context_config ctxCfg = ma_context_config_init();
    ctxCfg.threadPriority = ma_thread_priority_realtime;
    ctxCfg.alsa.useVerboseDeviceEnumeration = false;
    ctxCfg.pulse.tryAutoSpawn = true;
    ctxCfg.jack.tryStartServer = true;

    if (ma_context_init(backends.data(), backends.size(), &ctxCfg, &maCtx) != MA_SUCCESS) {
        L_FATAL("Failed to initialise miniaudio context");

        throw AudioException("Failed to initialise miniaudio context");
    }
    
    devs = new AudioDevices(&maCtx);
    analyser = new Analyser(&maCtx);

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
            hLayout5 = new QHBoxLayout;
            hLayout5->setSizeConstraint(QLayout::SetMaximumSize);
            hLayout5->setAlignment(Qt::AlignCenter);
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

            inputPause = new QPushButton;
            inputPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
           
            connect(inputPause, &QPushButton::clicked, [&]() { toggleAnalyser(); });

            hLayout2->addWidget(inputPause, 0, Qt::AlignCenter);
        }

        vLayout1->addSpacing(16);

        hLayout3 = new QHBoxLayout;
        vLayout1->addLayout(hLayout3);
        {
            settingsDock = new QDockWidget(tr("Settings"), this);
            settingsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto settingsDockWidget = new QWidget(settingsDock);
            settingsDock->setWidget(settingsDockWidget);

            fLayout4 = new QFormLayout;
            settingsDockWidget->setLayout(fLayout4);
            fLayout4->setSizeConstraint(QLayout::SetNoConstraint);
            {
                auto devWidget = new QWidget;
                auto devLayout = new QHBoxLayout(devWidget);
                devLayout->setSizeConstraint(QLayout::SetMaximumSize);
                devLayout->setContentsMargins(0, 0, 0, 0);
                {
                    inputDevIn = new QComboBox;
                    inputDevIn->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
                    inputDevIn->setMaximumWidth(150);

                    inputDevRefresh = new QPushButton;
                    inputDevRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
                    inputDevRefresh->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

                    connect(inputDevRefresh, &QPushButton::clicked,
                            [&]() { updateDevices(); });

                    devLayout->addWidget(inputDevIn, 0, Qt::AlignLeft);
                    devLayout->addWidget(inputDevRefresh, 0, Qt::AlignRight);
                }

                inputToggleSpectrum = new QCheckBox;

                connect(inputToggleSpectrum, &QCheckBox::toggled,
                        [&](const bool checked) { canvas->setDrawSpectrum(checked); });

                inputFftSize = new QComboBox;
                inputFftSize->addItems({"64", "128", "256", "512", "1024", "2048", "4096"});

                connect(inputFftSize, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                        [&](const QString value) { analyser->setFftSize(value.toInt()); });

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

                inputLpOrder = new QSpinBox;
                inputLpOrder->setRange(5, 22);

                connect(inputLpOrder, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser->setLinearPredictionOrder(value); });

                inputMaxFreq = new QSpinBox;
                inputMaxFreq->setRange(2500, 7000);
                inputMaxFreq->setStepType(QSpinBox::AdaptiveDecimalStepType);
                inputMaxFreq->setSuffix(" Hz");

                connect(inputMaxFreq, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser->setMaximumFrequency(value); });

                inputFreqScale = new QComboBox;
                inputFreqScale->addItems({"Linear", "Logarithmic", "Mel"});

                connect(inputFreqScale, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        [&](const int value) { canvas->setFrequencyScale(value); });

                inputFrameSpace = new QSpinBox;
                inputFrameSpace->setRange(5, 30);
                inputFrameSpace->setSingleStep(1);
                inputFrameSpace->setSuffix(" ms");

                connect(inputFrameSpace, QOverload<int>::of(&QSpinBox::valueChanged),
                        [&](const int value) { analyser->setFrameSpace(std::chrono::milliseconds(value)); });

                inputWindowSpan = new QDoubleSpinBox;
                inputWindowSpan->setRange(2, 30);
                inputWindowSpan->setSingleStep(0.5);
                inputWindowSpan->setSuffix(" s");

                connect(inputWindowSpan, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                        [&](const double value) { analyser->setWindowSpan(std::chrono::milliseconds(int(1000 * value))); });

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
                 
                inputPitchAlg = new QComboBox;
                inputPitchAlg->addItems({
                    "Wavelet",
                    "McLeod",
                    "YIN",
                    "AMDF",
                });

                connect(inputPitchAlg, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        [&](const int value) { analyser->setPitchAlgorithm(static_cast<PitchAlg>(value)); });

                inputFormantAlg = new QComboBox;
                inputFormantAlg->addItems({
                    "Linear prediction",
                    "Kalman filter",
                });

                connect(inputFormantAlg, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        [&](const int value) { analyser->setFormantMethod(static_cast<FormantMethod>(value)); });

                fLayout4->addRow(tr("Audio device:"), devWidget);
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

                fLayout4->addRow(tr("Pitch algorithm:"), inputPitchAlg);
                fLayout4->addRow(tr("Formant algorithm:"), inputFormantAlg);
            }

            addDockWidget(Qt::LeftDockWidgetArea, settingsDock);
            
            hLayout3->addWidget(canvas);
            canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        }
    }

    setWindowTitle(WINDOW_TITLE);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    loadSettings();

    updateDevices();
    analyser->startThread();

    connect(&timer, &QTimer::timeout, [&]() {
        updateFields();
        canvas->repaint();
    });
    timer.setTimerType(Qt::PreciseTimer);
    timer.start(1000.0 / 30.0);

    show();

}

MainWindow::~MainWindow() {
    delete central;
    
    delete devs;
    delete analyser;
    ma_context_uninit(&maCtx);
}

void MainWindow::closeEvent(QCloseEvent *event) {

    analyser->stopThread();
    all_fft_cleanup();

}

void MainWindow::updateFields() {

    int frame = canvas->getSelectedFrame();

    const auto & formants = analyser->getFormantFrame(frame);
    double pitch = analyser->getPitchFrame(frame);

    QPalette palette = this->palette();

    if (pitch > 0) {
        fieldPitch->setText(QString("%1 Hz").arg(pitch, 0, 'f', 1));
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
    const auto & inputs = devs->getInputs();
    const auto & outputs = devs->getOutputs();

    inputDevIn->disconnect();
    inputDevIn->clear();

    inputDevIn->addItem("", QVariant::fromValue(nullptr));

    for (const auto & dev : inputs) {
        const QString name = QString::fromLocal8Bit(dev.name.c_str());
        inputDevIn->addItem(name, QVariant::fromValue(std::make_pair(true, &dev.id)));
    }
   
    if (ma_context_is_loopback_supported(&maCtx)) {
        for (const auto & dev : outputs) { 
            const QString name = QString::fromLocal8Bit(dev.name.c_str());
            inputDevIn->addItem("(Loopback) " + name, QVariant::fromValue(std::make_pair(false, &dev.id)));
        }
    }

    inputDevIn->setCurrentIndex(0);

    connect(inputDevIn, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&](const int index) {
                if (index >= 0) {
                    const auto [ isInput, id ] = inputDevIn->itemData(index).value<DevicePair>();

                    if (isInput) {
                        analyser->setInputDevice(id);
                    }
                    else {
                        analyser->setOutputDevice(id);
                    }
                }
            });
}

void MainWindow::keyPressEvent(QKeyEvent * event) {
    const int key = event->key();

    if (key == Qt::Key_Escape) {
        close();
    }
    else if (key == Qt::Key_P) {
        toggleAnalyser();
    }
}

void MainWindow::toggleAnalyser() {
    analyser->toggle();

    bool running = analyser->isAnalysing();

    if (running) {
        inputPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
    else {
        inputPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}

void MainWindow::loadSettings()
{
    // Assume that analysis settings have already loaded and corrected if necessary.
   
    int nfft = analyser->getFftSize();
    int lpOrder = analyser->getLinearPredictionOrder();
    double maxFreq = analyser->getMaximumFrequency();
    int cepOrder = analyser->getCepstralOrder();
    double frameSpace = analyser->getFrameSpace().count();
    double windowSpan = analyser->getWindowSpan().count();
    PitchAlg pitchAlg = analyser->getPitchAlgorithm();
    FormantMethod formantAlg = analyser->getFormantMethod();

    // Assume that canvas settings have already loaded and corrected if necessary.
   
    int freqScale = canvas->getFrequencyScale();
    bool drawSpectrum = canvas->getDrawSpectrum();
    int minGain = canvas->getMinGainSpectrum();
    int maxGain = canvas->getMaxGainSpectrum();

    // Find the combobox index for nfft.
    int fftInd = inputFftSize->findText(QString::number(nfft));
    if (fftInd < 0) {
        fftInd = inputFftSize->findText("512");
    }

    QSettings settings;

#define callWithBlocker(obj, call) do { QSignalBlocker blocker(obj); (obj) -> call; } while (false)
 
    callWithBlocker(inputToggleSpectrum, setChecked(drawSpectrum));
    callWithBlocker(inputFftSize, setCurrentIndex(fftInd));
    callWithBlocker(inputLpOrder, setValue(lpOrder));
    callWithBlocker(inputMaxFreq, setValue(maxFreq));
    callWithBlocker(inputFreqScale, setCurrentIndex(freqScale));
    callWithBlocker(inputFrameSpace, setValue(frameSpace));
    callWithBlocker(inputWindowSpan, setValue(windowSpan));
    callWithBlocker(inputMinGain, setValue(minGain));
    callWithBlocker(inputMaxGain, setValue(maxGain));
    callWithBlocker(inputPitchAlg, setCurrentIndex(static_cast<int>(pitchAlg)));
    callWithBlocker(inputFormantAlg, setCurrentIndex(static_cast<int>(formantAlg)));

}
