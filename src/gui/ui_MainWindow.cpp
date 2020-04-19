#include "MainWindow.h"

QWidget * MainWindow::uiFields()
{
    auto container = new QWidget;
    auto parentLayout = new QHBoxLayout(container);

    auto widget = new QWidget(container);
    auto layout = this->fieldsLayout = new QBoxLayout(QBoxLayout::LeftToRight, widget);

    fieldOq = new QLineEdit;
    fieldOq->setReadOnly(true);
    layout->addWidget(fieldOq, 0, Qt::AlignCenter);

    for (int i = 0; i < numFormants; ++i) {
        auto field = new QLineEdit;
        field->setReadOnly(true);
        fieldFormant.push_back(field);
        layout->addWidget(field, 0, Qt::AlignCenter);
    }

    fieldPitch = new QLineEdit;
    fieldPitch->setReadOnly(true);
    layout->addWidget(fieldPitch, 0, Qt::AlignCenter);

    parentLayout->addWidget(widget, 0, Qt::AlignCenter);
    widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    return container;
}

#ifndef UI_BAR_FIELDS

QDockWidget * MainWindow::uiFieldsDock(QWidget * fields)
{
    auto dock = new QDockWidget("Estimates", this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

#ifdef UI_DOCK_FLOATABLE
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
#else
    dock->setFeatures(QDockWidget::DockWidgetMovable);
#endif

    fields->setParent(dock);
    dock->setWidget(fields);

    connect(dock, &QDockWidget::dockLocationChanged,
            [this] (Qt::DockWidgetArea area) {
                if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea) {
                    fieldsLayout->setDirection(QBoxLayout::TopToBottom);
                }
                else if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
                    fieldsLayout->setDirection(QBoxLayout::LeftToRight);
                }
            });

    connect(dock, &QDockWidget::topLevelChanged,
            [this] (bool topLevel) {
                if (topLevel) {
                    fieldsLayout->setDirection(QBoxLayout::LeftToRight);
                }
            });

    return dock;
}

#endif // UI_BAR_FIELDS

#ifdef UI_SHOW_SETTINGS

QWidget * MainWindow::uiAnalysisSettings()
{
    auto widget = new QWidget;
    auto layout = new QFormLayout(widget);
    layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->setLabelAlignment(Qt::AlignRight);

#ifdef UI_SHOW_DEVICE_SETTING

    auto devWidget = new QWidget;
    auto devLayout = new QHBoxLayout(devWidget);
    devLayout->setContentsMargins(0, 0, 0, 0);
    {
        this->inputDevice = new QComboBox;
        inputDevice->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        inputDevice->setMaximumWidth(150);

        auto refreshDevs = new QPushButton;
        refreshDevs->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        refreshDevs->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        connect(refreshDevs, &QPushButton::clicked, this, &MainWindow::updateDevices);

        devLayout->addWidget(inputDevice, 0, Qt::AlignLeft);
        devLayout->addWidget(refreshDevs, 0, Qt::AlignRight);
    }

    layout->addRow("Audio device:", devWidget);

#endif

    auto fftSize = new QComboBox;
    fftSize->addItems(availableFftSizes);
    // Special case for fft size.
    int fftInd = availableFftSizes.indexOf(QString::number(analyser->getFftSize()));
    if (fftInd < 0) {
        fftInd = availableFftSizes.indexOf("512");
    }
    fftSize->setCurrentIndex(fftInd);

    connect(fftSize, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) { analyser->setFftSize(availableFftSizes[index].toInt()); });

    layout->addRow("FFT size:", fftSize);

    auto lpOrder = new QSpinBox;
    lpOrder->setRange(4, 30);
    lpOrder->a_set(Value, LinearPredictionOrder);

    connect(lpOrder, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](const int value) { analyser->setLinearPredictionOrder(value); });
    
    layout->addRow("LP order:", lpOrder);

    auto maxFreq = new QSpinBox;
    maxFreq->setRange(2500, 7000);
    maxFreq->setStepType(QSpinBox::AdaptiveDecimalStepType);
    maxFreq->setSuffix(" Hz");
    maxFreq->a_set(Value, MaximumFrequency);

    connect(maxFreq, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](const int value) { analyser->setMaximumFrequency(value); });

    layout->addRow("Max frequency:", maxFreq);

#ifdef UI_SHOW_FRAME_SETTINGS

    auto frameLength = new QSpinBox;
    frameLength->setRange(25, 80);
    frameLength->setSingleStep(5);
    frameLength->setSuffix(" ms");
    frameLength->a_set_count(Value, FrameLength);

    connect(frameLength, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](const int value) { analyser->setFrameLength(std::chrono::milliseconds(value)); });

    layout->addRow("Frame length:", frameLength);

    auto frameSpace = new QSpinBox;
    frameSpace->setRange(1, 30);
    frameSpace->setSingleStep(1);
    frameSpace->setSuffix(" ms");
    frameSpace->a_set_count(Value, FrameSpace);

    connect(frameSpace, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](const int value) { analyser->setFrameSpace(std::chrono::milliseconds(value)); });

    layout->addRow("Frame space:", frameSpace);

#endif

    auto duration = new QDoubleSpinBox;
    duration->setRange(2, 30);
    duration->setSingleStep(0.5);
    duration->setSuffix(" s");
    duration->a_set_count(Value, WindowSpan);

    connect(duration, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](const double value) { analyser->setWindowSpan(std::chrono::milliseconds(int(1000 * value))); });

    layout->addRow("Duration:", duration);

    auto pitchAlg = new QComboBox;
    pitchAlg->addItems(availablePitchAlgs);
    pitchAlg->a_set_enum(CurrentIndex, PitchAlgorithm);

    connect(pitchAlg, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](const int index) { analyser->setPitchAlgorithm(static_cast<PitchAlg>(index)); });

    layout->addRow("Pitch algorithm:", pitchAlg);

    auto formantAlg = new QComboBox;
    formantAlg->addItems(availableFormantAlgs);
    formantAlg->a_set_enum(CurrentIndex, FormantMethod);

    connect(formantAlg, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](const int index) { analyser->setFormantMethod(static_cast<FormantMethod>(index)); });

    layout->addRow("Formant algorithm:", formantAlg);

    return widget;
}

QWidget * MainWindow::uiDisplaySettings()
{
    auto widget = new QWidget;
    auto layout = new QFormLayout(widget);
    layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->setLabelAlignment(Qt::AlignRight);

    auto toggleSpectrogram = new QCheckBox;
    toggleSpectrogram->c_set(Checked, DrawSpectrum);

    connect(toggleSpectrogram, &QCheckBox::toggled,
            [this](const bool checked) { canvas->setDrawSpectrum(checked); });

    layout->addRow("Toggle spectrogram:", toggleSpectrogram);

    auto toggleTracks = new QCheckBox;
    toggleTracks->c_set(Checked, DrawTracks);

    connect(toggleTracks, &QCheckBox::toggled,
            [this](const bool checked) { canvas->setDrawTracks(checked); });

    layout->addRow("Toggle tracks:", toggleTracks);

    auto minGain = new QSpinBox;
    minGain->setRange(-180, 60);
    minGain->setSingleStep(10);
    minGain->setSuffix(" dB");
    minGain->c_set(Value, MinGainSpectrum);

    auto maxGain = new QSpinBox;
    maxGain->setRange(-180, 60);
    maxGain->setSingleStep(10);
    maxGain->setSuffix(" dB");
    maxGain->c_set(Value, MaxGainSpectrum);

    connect(minGain, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, maxGain] (const int value) { canvas->setMinGainSpectrum(value);
                                                maxGain->setMinimum(value + 10); });

    connect(maxGain, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, minGain] (const int value) { canvas->setMaxGainSpectrum(value);
                                                minGain->setMaximum(value - 10); });

    layout->addRow("Min gain:", minGain);
    layout->addRow("Max gain:", maxGain);

    auto freqScale = new QComboBox;
    freqScale->addItems(availableFreqScales);
    freqScale->c_set(CurrentIndex, FrequencyScale);

    connect(freqScale, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this] (const int value) { canvas->setFrequencyScale(value); });

    layout->addRow("Frequency scale:", freqScale);

    auto pitchThick = new QSpinBox;
    pitchThick->setRange(1, 20);
    pitchThick->setSingleStep(1);
    pitchThick->setSuffix(" px");
    pitchThick->c_set(Value, PitchThickness);
    
    connect(pitchThick, QOverload<int>::of(&QSpinBox::valueChanged),
            [this] (const int value) { canvas->setPitchThickness(value); });

    layout->addRow("Pitch thickness:", pitchThick);

    this->pitchColor = new QPushButton;
    auto pitchColorDialog = new QColorDialog(canvas->getPitchColor(), this);
    pitchColorDialog->setWindowTitle("Select pitch color");
    pitchColorDialog->setOption(QColorDialog::NoButtons);

    connect(pitchColor, &QPushButton::clicked, pitchColorDialog, &QColorDialog::show);
    connect(pitchColorDialog, &QColorDialog::currentColorChanged,
            [this] (const QColor& c) {
                canvas->setPitchColor(c);
                updateColorButtons();
            });

    layout->addRow("Pitch colour:", pitchColor);

    auto formantThick = new QSpinBox;
    formantThick->setRange(1, 20);
    formantThick->setSingleStep(1);
    formantThick->setSuffix(" px");
    formantThick->c_set(Value, FormantThickness);
    
    connect(formantThick, QOverload<int>::of(&QSpinBox::valueChanged),
            [this] (const int value) { canvas->setFormantThickness(value); });

    layout->addRow("Formant thickness:", formantThick);

    for (int i = 0; i < numFormants; ++i) {
        this->formantColors[i] = new QPushButton;
        auto formantColorDialog = new QColorDialog(canvas->getFormantColor(i), this);
        formantColorDialog->setWindowTitle(QString("Select F%1 color").arg(i + 1));
        formantColorDialog->setOption(QColorDialog::NoButtons);

        connect(formantColors[i], &QPushButton::clicked, formantColorDialog, &QColorDialog::show);
        connect(formantColorDialog, &QColorDialog::currentColorChanged,
                [this, i] (const QColor &c) {
                    canvas->setFormantColor(i, c);
                    updateColorButtons();
                });

        layout->addRow(QString("F%1 color").arg(i + 1), formantColors[i]);
    }

    auto colorMap = new QComboBox;
    colorMap->addItems(availableColorMaps);
    colorMap->c_set(CurrentText, SpectrumColor);

    connect(colorMap, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this] (int index) { canvas->setSpectrumColor(availableColorMaps[index]); });

    layout->addRow("Spectrogram colors:", colorMap);

#ifdef UI_DISPLAY_SETTINGS_IN_DIALOG
    this->displaySettings = widget;
#endif

    updateColorButtons();

    return widget;
}

QDockWidget * MainWindow::uiSettingsDock(QWidget * analysisSettings, QWidget * displaySettings)
{
    auto dock = new QDockWidget("Settings", this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

#ifdef UI_DOCK_FLOATABLE
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
#else
    dock->setFeatures(QDockWidget::DockWidgetMovable);
#endif

    auto container = new QWidget;
    dock->setWidget(container);
    
    auto layout = new QVBoxLayout(container);
    container->setMinimumWidth(150);

    layout->addWidget(analysisSettings);
   
    layout->addSpacing(4);

    auto separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);

    layout->addSpacing(4);

#ifdef UI_DISPLAY_SETTINGS_IN_DIALOG
    auto openDisplaySettings = new QPushButton("Open display settings");
    connect(openDisplaySettings, &QPushButton::clicked, displaySettings, &QWidget::showNormal);
    layout->addWidget(openDisplaySettings);
#else
    layout->addWidget(displaySettings);
#endif

    return dock;
}

#endif // UI_SHOW_SETTINGS

QWidget * MainWindow::uiBarLeft()
{
    auto widget = new QWidget;
    widget->setContentsMargins(0, 0, 0, 0);
    auto layout = new QHBoxLayout(widget);

    // Nothing yet.
    (void) layout;
    
    return widget;
}

QWidget * MainWindow::uiBarCenter(QWidget * fields)
{
    auto widget = new QWidget;
    widget->setContentsMargins(0, 0, 0, 0);
    auto layout = new QHBoxLayout(widget);

#ifdef UI_BAR_FIELDS
    layout->addWidget(fields);
#endif

    return widget;
}

QWidget * MainWindow::uiBarRight()
{
    auto widget = new QWidget;
    widget->setContentsMargins(0, 0, 0, 0);
    auto layout = new QHBoxLayout(widget);

#ifdef UI_BAR_LINKS
    constexpr int buttonSize = 32;

    auto github = new QPushButton;
    github->setFixedSize(buttonSize, buttonSize);
    github->setStyleSheet("QPushButton { border-image: url(:/icons/github.png) 0 0 0 0 stretch stretch; border: none; }");
    github->setCursor(Qt::PointingHandCursor);

    connect(github, &QPushButton::clicked, [&]() {
                QDesktopServices::openUrl(QUrl("https://www.github.com/ichi-rika/speech-analysis"));
            });

    layout->addWidget(github);
    layout->addSpacing(8);

    auto patreon = new QPushButton;
    patreon->setFixedSize(buttonSize, buttonSize);
    patreon->setStyleSheet("QPushButton { border-image: url(:/icons/patreon.png) 0 0 0 0 stretch stretch; border: none; }");
    patreon->setCursor(Qt::PointingHandCursor);

    connect(patreon, &QPushButton::clicked, [&]() {         
                QDesktopServices::openUrl(QUrl("https://www.patreon.com/cloyunhee"));
            });

    layout->addWidget(patreon);
    layout->addSpacing(8);
#endif // UI_BAR_LINKS

#ifdef UI_BAR_PAUSE
    this->pause = new QPushButton;
    pause->setFixedSize(buttonSize, buttonSize);
    pause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
   
    connect(pause, &QPushButton::clicked, this, &MainWindow::toggleAnalyser);

    layout->addWidget(pause);
#endif

#ifdef UI_BAR_FULLSCREEN
    this->fullscreen = new QPushButton("F");
    fullscreen->setFixedSize(buttonSize, buttonSize);
    fullscreen->setStyleSheet("QPushButton { padding: 0; font-weight: bold; }");
    fullscreen->setCheckable(true);

    connect(fullscreen, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);

    layout->addWidget(fullscreen);
#endif

#ifdef UI_BAR_SETTINGS
    constexpr int buttonSize = 100;

    auto settings = new QPushButton;
;
    settings->setFixedSize(buttonSize, buttonSize);
    settings->setStyleSheet("QPushButton { border-image: url(:/icons/settings.png) 0 0 0 0 stretch stretch; border: none; }");

    connect(settings, &QPushButton::clicked, this, &MainWindow::openSettings);

    layout->addWidget(settings);
#endif
    
    return widget;
}

QWidget * MainWindow::uiBar(QWidget * fields)
{
    auto widget = new QWidget;
    auto layout = new QHBoxLayout(widget);

#ifdef UI_HAS_LEFT_BAR
    layout->addWidget(uiBarLeft(), 0, Qt::AlignLeft);
#endif

#ifdef UI_HAS_CENTER_BAR
    layout->addWidget(uiBarCenter(fields), 0, Qt::AlignCenter);
#endif

#ifdef UI_HAS_RIGHT_BAR
    layout->addWidget(uiBarRight(), 0, Qt::AlignRight);
#endif

    return widget;
}
