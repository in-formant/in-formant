//
// Created by rika on 06/12/2019.
//

#include <cstdio>
#include "MainWindow.h"
#include "ColorMaps.h"
#include "FFT/FFT.h"
#include "../log/simpleQtLogger.h"
#include "rpmalloc.h"
#ifdef Q_OS_ANDROID
    #include <QtAndroid>
    #include <QAndroidIntent>
    #include <jni.h>
    #include "../jni/JniInstance.h"
#endif
#ifdef Q_OS_WASM
    #include <emscripten/html5.h>
#endif

constexpr int maxWidthComboBox = 200;

MainWindow::MainWindow()
    : timer(this)
{
    buildColorMaps();

    for (int nfft = 64; nfft <= 2048; nfft *= 2) {
        availableFftSizes << QString::number(nfft);
    }

    availablePitchAlgs
        << "Wavelet"
        << "McLeod"
        << "YIN"
        << "AMDF";

    availableFormantAlgs
        << "LP"
        << "KARMA";

    availableFreqScales
        << "Linear"
        << "Logarithmic"
        << "Mel";

    for (auto & [name, map] : colorMaps) {
        (void) map;
        availableColorMaps << name;
    }

    L_INFO("Initialising miniaudio context...");

    rpm::vector<ma_backend> backends{
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
#ifndef Q_OS_WASM
    ctxCfg.allocationCallbacks.onMalloc = [](size_t sz, void *) { return rpmalloc(sz); };
    ctxCfg.allocationCallbacks.onRealloc = [](void *p, size_t sz, void *) { return rprealloc(p, sz); };
    ctxCfg.allocationCallbacks.onFree = [](void *p, void *) { return rpfree(p); };
#endif

    if (ma_context_init(backends.data(), backends.size(), &ctxCfg, &maCtx) != MA_SUCCESS) {
        L_FATAL("Failed to initialise miniaudio context");

        throw AudioException("Failed to initialise miniaudio context");
    }
    
    devs = new AudioDevices(&maCtx);

    sineWave = new SineWave();
    noiseFilter = new NoiseFilter();

#if defined(Q_OS_MAC)
    audioInterfaceMem = malloc(sizeof(AudioInterface));
    audioInterface = new (audioInterfaceMem) AudioInterface(&maCtx, sineWave, noiseFilter);
#else
    audioInterface = new AudioInterface(&maCtx, sineWave, noiseFilter);
#endif

    analyser = new Analyser(audioInterface);

    canvas = new AnalyserCanvas(analyser, sineWave, noiseFilter);
    powerSpectrum = new PowerSpectrum(analyser, canvas);

#ifdef Q_OS_ANDROID
    JniInstance::createInstance(analyser, canvas, powerSpectrum);
#endif 

    auto fields = uiFields();
    
#ifndef UI_BAR_FIELDS
    addDockWidget(Qt::TopDockWidgetArea, uiFieldsDock(fields));
#endif

#ifdef UI_SHOW_SETTINGS
    auto analysisSettings = uiAnalysisSettings();
    auto displaySettings = uiDisplaySettings();

#ifdef UI_DISPLAY_SETTINGS_IN_DIALOG
    displaySettings->setParent(this);
    displaySettings->setWindowFlag(Qt::Tool);
    displaySettings->setVisible(false);
    displaySettings->installEventFilter(this);
#endif // UI_DISPLAY_SETTINGS_IN_DIALOG

    addDockWidget(Qt::LeftDockWidgetArea, uiSettingsDock(analysisSettings, displaySettings));
#endif // UI_SHOW_SETTINGS

    auto central = new QWidget;
    setCentralWidget(central);
    auto layout = new QVBoxLayout(central);
    {
        layout->addWidget(uiBar(fields));

        auto splitter = new QSplitter;
        splitter->setHandleWidth(12);
       
        auto splitterLayout = new QHBoxLayout(splitter);
        { 
            splitterLayout->addWidget(canvas, 4);
            canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            splitterLayout->addWidget(powerSpectrum, 1);
            powerSpectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
        layout->addWidget(splitter);
    }

    setWindowTitle(WINDOW_TITLE);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    window()->installEventFilter(this);

#if DO_UPDATE_DEVICES
    updateDevices();
#endif

    connect(this, &MainWindow::newFramesTracks, canvas, &AnalyserCanvas::renderTracks);
    connect(this, &MainWindow::newFramesSpectrum, canvas, &AnalyserCanvas::renderSpectrogram);
    connect(this, &MainWindow::newFramesSpectrum, powerSpectrum, &PowerSpectrum::renderSpectrum);
    connect(this, &MainWindow::newFramesLpc, powerSpectrum, &PowerSpectrum::renderLpc);
    connect(this, &MainWindow::newFramesUI, canvas, &AnalyserCanvas::renderScaleAndCursor);
   
    connect(&timer, &QTimer::timeout, [&]() {
        analyser->callIfNewFrames(
                    [this](auto&&... ts) { emit newFramesTracks(std::forward<decltype(ts)>(ts)...); },
                    [this](auto&&... ts) { emit newFramesSpectrum(std::forward<decltype(ts)>(ts)...); },
                    [this](auto&&... ts) { emit newFramesLpc(std::forward<decltype(ts)>(ts)...); },
                    [this](auto&&... ts) { emit newFramesUI(std::forward<decltype(ts)>(ts)...); }
        );
        updateFields();
        canvas->repaint();
        powerSpectrum->repaint();
    });

#if TIMER_SLOWER
    timer.setTimerType(Qt::CoarseTimer);
    timer.start(1000.0 / 30.0);
#else
    timer.setTimerType(Qt::PreciseTimer);
    timer.start(1000.0 / 60.0);
#endif

    analyser->startThread();

    show();
}

MainWindow::~MainWindow()
{
    if (canvas != nullptr) {
        cleanup();
    }
}

void MainWindow::cleanup()
{
    timer.stop();

    while (timer.isActive()) {
        std::this_thread::yield();
    }

    delete canvas;
    delete powerSpectrum;

    delete analyser;

#if defined(Q_OS_MAC)
    audioInterface->~AudioInterface();
    free(audioInterfaceMem);
#else
    delete audioInterface;
#endif

    delete sineWave;

    delete devs;
    ma_context_uninit(&maCtx);

    all_fft_cleanup();

    canvas = nullptr;
}

void MainWindow::updateFields() {

    const int frame = canvas->getSelectedFrame();

    const auto & formants = analyser->getFormantFrame(frame);
    const double pitch = analyser->getPitchFrame(frame);
    const double Oq = analyser->getOqFrame(frame);

    QPalette palette = this->palette();

    fieldOq->setText(QString("Oq = %1").arg(Oq));

    for (int i = 0; i < numFormants; ++i) {
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

#if DO_UPDATE_DEVICES

void MainWindow::updateDevices()
{
    const auto & inputs = devs->getInputs();
    const auto & outputs = devs->getOutputs();

    QSignalBlocker blocker(inputDevice);

    inputDevice->clear();
    inputDevice->addItem("Default input device", QVariant::fromValue(std::make_pair(true, (const ma_device_id *) nullptr)));

    for (const auto & dev : inputs) {
        const QString name = QString::fromLocal8Bit(dev.name.c_str());
        inputDevice->addItem(name, QVariant::fromValue(std::make_pair(true, &dev.id)));
    }
   
    if (ma_context_is_loopback_supported(&maCtx)) {
        for (const auto & dev : outputs) { 
            const QString name = QString::fromLocal8Bit(dev.name.c_str());
            inputDevice->addItem("(Loopback) " + name, QVariant::fromValue(std::make_pair(false, &dev.id)));
        }
    }

    inputDevice->setCurrentIndex(0);

    connect(inputDevice, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&](const int index) {
                if (index >= 0) {
                    const auto [ isInput, id ] = inputDevice->itemData(index).value<DevicePair>();

                    if (isInput || id == nullptr) {
                        analyser->setInputDevice(id);
                    }
                    else {
                        analyser->setOutputDevice(id);
                    }
                }
            });
}

#endif

#ifdef UI_SHOW_SETTINGS

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

    pitchColor->setStyleSheet(css.arg(canvas->getPitchColor().name()));
    
    for (int i = 0; i < numFormants; ++i) {
        formantColors[i]->setStyleSheet(css.arg(canvas->getFormantColor(i).name()));
    }

}

#endif

void MainWindow::toggleAnalyser() {
    bool running = analyser->isAnalysing();

    analyser->toggle(!running);

#if defined(UI_BAR_PAUSE_LEFT) || defined(UI_BAR_PAUSE_RIGHT)
    if (running) {
        pause->setStyleSheet(stylePlay);
    } else {
        pause->setStyleSheet(stylePause);
    }
#endif
}

void MainWindow::toggleFullscreen() {
    bool isFullscreen = (windowState() == Qt::WindowFullScreen);

    if (isFullscreen) {
        setWindowState(Qt::WindowNoState);
    }
    else {
        setWindowState(Qt::WindowFullScreen);
    }

#ifdef UI_BAR_FULLSCREEN
    fullscreen->setChecked(!isFullscreen);
#endif
}

bool MainWindow::eventFilter(QObject * obj, QEvent * event)
{
    if (event->type() == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();

        if (obj == window()) {
            if (key == Qt::Key_Escape) {
                close();
                return true;
            }
            else if (key == Qt::Key_P) {
                toggleAnalyser();
                return true;
            }
            else if (key == Qt::Key_F) {
                toggleFullscreen();
                return true;
            }
        }
#ifdef UI_DISPLAY_SETTINGS_IN_DIALOG
        else if (obj == displaySettings) {
            if (key == Qt::Key_Escape) {
                displaySettings->setVisible(false);
                window()->activateWindow();
                window()->setFocus(Qt::ActiveWindowFocusReason);
                return true;
            }
        }
#endif
    }
        
    return QObject::eventFilter(obj, event);
}

#ifdef UI_BAR_SETTINGS
void MainWindow::openSettings()
{
    QAndroidIntent intent(QtAndroid::androidActivity().object(), "fr.cloyunhee.speechanalysis.SettingsActivity");
    QtAndroid::startActivity(intent, 0);
}
#endif

void MainWindow::closeEvent(QCloseEvent * event)
{
    cleanup();
    qApp->quit();
}

void MainWindow::saveSettings() 
{
    analyser->saveSettings();
    canvas->saveSettings();
}
