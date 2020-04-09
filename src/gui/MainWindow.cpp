//
// Created by rika on 06/12/2019.
//

#include "MainWindow.h"
#include "ColorMaps.h"
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
    powerSpectrum = new PowerSpectrum(analyser);

    fieldsDock = new QDockWidget(tr("Estimates"), this);
    {
        fieldsDock->setAllowedAreas(Qt::AllDockWidgetAreas);
        fieldsDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

        auto dockWidget = new QWidget(fieldsDock);
        fieldsDock->setWidget(dockWidget);

        auto ly1 = new QBoxLayout(QBoxLayout::LeftToRight, dockWidget);

        connect(fieldsDock, &QDockWidget::dockLocationChanged,
                [&, ly1](Qt::DockWidgetArea area) {
                    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea) {
                        ly1->setDirection(QBoxLayout::TopToBottom);
                    }
                    else if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
                        ly1->setDirection(QBoxLayout::LeftToRight);
                    }
                });
        
        connect(fieldsDock, &QDockWidget::topLevelChanged,
                [&, ly1](bool topLevel) {
                    if (topLevel) {
                        ly1->setDirection(QBoxLayout::LeftToRight);
                    }
                });
        
        fieldOq = new QLineEdit;
        fieldOq->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        fieldOq->setReadOnly(true);

        ly1->addWidget(fieldOq, 0, Qt::AlignCenter);
        
        for (int i = 0; i < inputFormantColor.size(); ++i) {
            auto field = new QLineEdit;

            field->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            field->setReadOnly(true);

            fieldFormant.push_back(field);

            ly1->addWidget(field, 0, Qt::AlignCenter);
        }  

        fieldPitch = new QLineEdit;
        fieldPitch->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        fieldPitch->setReadOnly(true);

        ly1->addWidget(fieldPitch, 0, Qt::AlignCenter);
    }

    settingsDock = new QDockWidget(tr("Settings"), this);
    {
        settingsDock->setAllowedAreas(Qt::AllDockWidgetAreas);
        settingsDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

        auto dockScroll = new QScrollArea;
        auto dockWidget = new QWidget(settingsDock);
        settingsDock->setWidget(dockScroll);
        
        auto ly1 = new QFormLayout(dockWidget);
        ly1->setSizeConstraint(QLayout::SetNoConstraint);
        {
            auto devWidget = new QWidget;
            auto devLayout = new QHBoxLayout(devWidget);
            devLayout->setSizeConstraint(QLayout::SetMaximumSize);
            devLayout->setContentsMargins(0, 0, 0, 0);
            {
                inputDevIn = new QComboBox;
                inputDevIn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
                inputDevIn->setMaximumWidth(150);

                inputDevRefresh = new QPushButton;
                inputDevRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
                inputDevRefresh->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

                connect(inputDevRefresh, &QPushButton::clicked,
                        [&]() { updateDevices(); });

                devLayout->addWidget(inputDevIn, 0, Qt::AlignLeft);
                devLayout->addWidget(inputDevRefresh, 0, Qt::AlignRight);
            }

            inputDisplayDialog = new QPushButton("Open dialog");
        
            connect(inputDisplayDialog, &QPushButton::clicked,
                    [&]() { dialogDisplay->setVisible(true); });

            inputFftSize = new QComboBox;
            inputFftSize->addItems({"64", "128", "256", "512", "1024", "2048"});

            connect(inputFftSize, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                    [&](const QString value) { analyser->setFftSize(value.toInt()); });

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

            inputFrameLength = new QSpinBox;
            inputFrameLength->setRange(25, 80);
            inputFrameLength->setSingleStep(5);
            inputFrameLength->setSuffix(" ms");

            connect(inputFrameLength, QOverload<int>::of(&QSpinBox::valueChanged),
                    [&](const int value) { analyser->setFrameLength(std::chrono::milliseconds(value)); });

            inputFrameSpace = new QSpinBox;
            inputFrameSpace->setRange(1, 30);
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

            ly1->addRow(tr("Audio device:"), devWidget);
            ly1->addRow(tr("Display settings:"), inputDisplayDialog);
            ly1->addRow(tr("FFT size:"), inputFftSize);
            ly1->addRow(tr("Linear prediction order:"), inputLpOrder);
            ly1->addRow(tr("Maximum frequency:"), inputMaxFreq);
            ly1->addRow(tr("Frame length:"), inputFrameLength);
            ly1->addRow(tr("Frame space:"), inputFrameSpace);
            ly1->addRow(tr("Analysis duration:"), inputWindowSpan);

            ly1->addRow(tr("Pitch algorithm:"), inputPitchAlg);
            ly1->addRow(tr("Formant algorithm:"), inputFormantAlg);
        }
        
        dockScroll->setWidget(dockWidget);
    }
   
    dialogDisplay = new QWidget(window(), Qt::Tool);
    {
        auto ly1 = new QFormLayout(dialogDisplay);
        
        inputToggleSpectrum = new QCheckBox;

        connect(inputToggleSpectrum, &QCheckBox::toggled,
                [&](const bool checked) { canvas->setDrawSpectrum(checked); });

        inputToggleTracks = new QCheckBox;

        connect(inputToggleTracks, &QCheckBox::toggled,
                [&](const bool checked) { canvas->setDrawTracks(checked); });

        inputMinGain = new QSpinBox;
        inputMinGain->setRange(-200, 60);
        inputMinGain->setSingleStep(10);
        inputMinGain->setSuffix(" dB");

        connect(inputMinGain, QOverload<int>::of(&QSpinBox::valueChanged),
                [&](const int value) { canvas->setMinGainSpectrum(value);
                                       inputMaxGain->setMinimum(value + 10); });

        inputMaxGain = new QSpinBox;
        inputMaxGain->setRange(-200, 60);
        inputMaxGain->setSingleStep(10);
        inputMaxGain->setSuffix(" dB");

        connect(inputMaxGain, QOverload<int>::of(&QSpinBox::valueChanged),
                [&](const int value) { canvas->setMaxGainSpectrum(value);
                                       inputMinGain->setMaximum(value - 10); });

        inputFreqScale = new QComboBox;
        inputFreqScale->addItems({"Linear", "Logarithmic", "Mel"});

        connect(inputFreqScale, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](const int value) { canvas->setFrequencyScale(value); });

        inputPitchThick = new QSpinBox;
        inputPitchThick->setRange(1, 20);
        inputPitchThick->setSingleStep(1);
        inputPitchThick->setSuffix(" px");
        
        connect(inputPitchThick, QOverload<int>::of(&QSpinBox::valueChanged),
                [&](const int value) { canvas->setPitchThickness(value); });

        inputPitchColor = new QPushButton;

        connect(inputPitchColor, &QPushButton::clicked,
                [=] () {
                    QColor c = QColorDialog::getColor(
                        canvas->getPitchColor(),
                        this,
                        QString("Select pitch color"),
                        QColorDialog::DontUseNativeDialog
                    );
                    if (c.isValid()) {
                        canvas->setPitchColor(c);
                        updateColorButtons();
                    }
                });

        inputFormantThick = new QSpinBox;
        inputFormantThick->setRange(1, 20);
        inputFormantThick->setSingleStep(1);
        inputFormantThick->setSuffix(" px");
        
        connect(inputFormantThick, QOverload<int>::of(&QSpinBox::valueChanged),
                [&](const int value) { canvas->setFormantThickness(value); });

        for (int nb = 0; nb < inputFormantColor.size(); ++nb) {
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
                            updateColorButtons();
                        }
                    });

            inputFormantColor[nb] = input;
        }

        inputColorMap = new QComboBox;
        for (auto & [name, map] : colorMaps) {
            inputColorMap->addItem(name);
        }

        connect(inputColorMap, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                [&](const QString & name) { canvas->setSpectrumColor(name); });

        ly1->addRow(tr("Show spectrum:"), inputToggleSpectrum);
        ly1->addRow(tr("Show tracks:"), inputToggleTracks);
        ly1->addRow(tr("Minimum gain:"), inputMinGain);
        ly1->addRow(tr("Maximum gain:"), inputMaxGain);
        ly1->addRow(tr("Frequency scale:"), inputFreqScale);

        ly1->addRow(tr("Pitch thickness:"), inputPitchThick);
        ly1->addRow(tr("Pitch color:"), inputPitchColor);

        ly1->addRow(tr("Formant thickness:"), inputFormantThick);
        for (int nb = 0; nb < inputFormantColor.size(); ++nb) {
            const QString labelStr = QString("F%1 color:").arg(nb + 1);
            ly1->addRow(tr(qPrintable(labelStr)), inputFormantColor[nb]);
        }

        ly1->addRow(tr("Spectrum color map:"), inputColorMap);
    }

    auto ly1 = new QVBoxLayout(central);
    {
        auto ly2 = new QHBoxLayout;
        ly1->addLayout(ly2);
        {
            auto w1 = new QWidget;
            w1->setContentsMargins(0, 0, 0, 0);
            auto ly3 = new QHBoxLayout(w1);
            {
            }

            auto w2 = new QWidget;
            w2->setContentsMargins(0, 0, 0, 0);
            auto ly4 = new QHBoxLayout(w2);
            {
                auto github = new QPushButton;
                github->setFixedSize(30, 30);
                github->setStyleSheet("QPushButton { border-image: url(:/icons/github.png) 0 0 0 0 stretch stretch; border: none; }");
                github->setCursor(Qt::PointingHandCursor);

                connect(github, &QPushButton::clicked, [&]() {
                            QDesktopServices::openUrl(QUrl("https://www.github.com/ichi-rika/speech-analysis"));
                        });

                auto patreon = new QPushButton;
                patreon->setFixedSize(30, 30);
                patreon->setStyleSheet("QPushButton { border-image: url(:/icons/patreon.png) 0 0 0 0 stretch stretch; border: none; }");
                patreon->setCursor(Qt::PointingHandCursor);

                connect(patreon, &QPushButton::clicked, [&]() {
                            QDesktopServices::openUrl(QUrl("https://www.patreon.com/cloyunhee"));
                        });

                inputPause = new QPushButton;
                inputPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
                inputPause->setFixedSize(30, 30);
               
                connect(inputPause, &QPushButton::clicked, [&]() { toggleAnalyser(); });

                inputFullscreen = new QPushButton("F");
                inputFullscreen->setCheckable(true);
                inputFullscreen->setStyleSheet("padding: 0; font-weight: bold;");
                inputFullscreen->setFixedSize(30, 30);

                connect(inputFullscreen, &QPushButton::clicked, [&]() { toggleFullscreen(); });

                ly4->addWidget(github);
                ly4->addSpacing(8);
                ly4->addWidget(patreon);
                ly4->addSpacing(8);
                ly4->addWidget(inputPause);
                ly4->addWidget(inputFullscreen);
            }

            ly2->addWidget(w1, 0, Qt::AlignLeft);
            ly2->addWidget(w2, 0, Qt::AlignRight);
        }

        auto ly3 = new QHBoxLayout;
        ly1->addLayout(ly3);
        { 
            ly3->addWidget(canvas, 3);
            canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            ly3->addWidget(powerSpectrum, 1);
            powerSpectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }
 
    addDockWidget(Qt::TopDockWidgetArea, fieldsDock);
    addDockWidget(Qt::LeftDockWidgetArea, settingsDock);

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

    all_fft_cleanup();
}

void MainWindow::updateFields() {

    const int frame = canvas->getSelectedFrame();

    const auto & formants = analyser->getFormantFrame(frame);
    const double pitch = analyser->getPitchFrame(frame);
    const double Oq = analyser->getOqFrame(frame);


    QPalette palette = this->palette();

    fieldOq->setText(QString("Oq = %1").arg(Oq));

    for (int i = 0; i < inputFormantColor.size(); ++i) {
        if (i < formants.nFormants) {
            fieldFormant[i]->setText(QString("F%1 = %2 Hz").arg(i + 1).arg(round(formants.formant[i].frequency)));
        }
        else {
            fieldFormant[i]->setText("");
        }
    }

    if (pitch > 0) {
        fieldPitch->setText(QString("H1 = %1 Hz").arg(pitch, 0, 'f', 1));
    } else {
        fieldPitch->setText("Unvoiced");
    }
    fieldPitch->adjustSize();

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

void MainWindow::updateColorButtons() {
    static QString css = QStringLiteral(" \
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
                        ");

    inputPitchColor->setStyleSheet(css.arg(canvas->getPitchColor().name()));
    
    for (int i = 0; i < inputFormantColor.size(); ++i) {
        inputFormantColor[i]->setStyleSheet(css.arg(canvas->getFormantColor(i).name()));
    }

}

void MainWindow::keyPressEvent(QKeyEvent * event) {
    const int key = event->key();

    if (key == Qt::Key_Escape) {
        close();
    }
    else if (key == Qt::Key_P) {
        toggleAnalyser();
    }
    else if (key == Qt::Key_F) {
        toggleFullscreen();
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

void MainWindow::toggleFullscreen() {
    bool isFullscreen = (windowState() == Qt::WindowFullScreen);

    if (isFullscreen) {
        inputFullscreen->setChecked(false);
        setWindowState(Qt::WindowNoState);
    }
    else {
        inputFullscreen->setChecked(true);
        setWindowState(Qt::WindowFullScreen);
    }
}

void MainWindow::loadSettings()
{
    // Assume that analysis settings have already loaded and corrected if necessary.
   
    int nfft = analyser->getFftSize();
    int lpOrder = analyser->getLinearPredictionOrder();
    double maxFreq = analyser->getMaximumFrequency();
    int cepOrder = analyser->getCepstralOrder();
    double frameLength = analyser->getFrameLength().count();
    double frameSpace = analyser->getFrameSpace().count();
    double windowSpan = analyser->getWindowSpan().count();
    PitchAlg pitchAlg = analyser->getPitchAlgorithm();
    FormantMethod formantAlg = analyser->getFormantMethod();

    // Assume that canvas settings have already loaded and corrected if necessary.
   
    int freqScale = canvas->getFrequencyScale();
    bool drawSpectrum = canvas->getDrawSpectrum();
    bool drawTracks = canvas->getDrawTracks();
    int minGain = canvas->getMinGainSpectrum();
    int maxGain = canvas->getMaxGainSpectrum();
    int pitchThick = canvas->getPitchThickness();
    int formantThick = canvas->getFormantThickness();
    QString colorMapName = canvas->getSpectrumColor();

    // Find the combobox index for nfft.
    int fftInd = inputFftSize->findText(QString::number(nfft));
    if (fftInd < 0) {
        fftInd = inputFftSize->findText("512");
    }

#define callWithBlocker(obj, call) do { QSignalBlocker blocker(obj); (obj) -> call; } while (false)
 
    callWithBlocker(inputFftSize, setCurrentIndex(fftInd));
    callWithBlocker(inputLpOrder, setValue(lpOrder));
    callWithBlocker(inputMaxFreq, setValue(maxFreq));
    callWithBlocker(inputFrameLength, setValue(frameLength));
    callWithBlocker(inputFrameSpace, setValue(frameSpace));
    callWithBlocker(inputWindowSpan, setValue(windowSpan));
    callWithBlocker(inputPitchAlg, setCurrentIndex(static_cast<int>(pitchAlg)));
    callWithBlocker(inputFormantAlg, setCurrentIndex(static_cast<int>(formantAlg)));

    callWithBlocker(inputToggleSpectrum, setChecked(drawSpectrum));
    callWithBlocker(inputToggleTracks, setChecked(drawTracks));
    callWithBlocker(inputFreqScale, setCurrentIndex(freqScale));
    callWithBlocker(inputMinGain, setValue(minGain));
    callWithBlocker(inputMaxGain, setValue(maxGain));
    callWithBlocker(inputPitchThick, setValue(pitchThick));
    callWithBlocker(inputFormantThick, setValue(formantThick));

    updateColorButtons();

    callWithBlocker(inputColorMap, setCurrentText(colorMapName));

}
