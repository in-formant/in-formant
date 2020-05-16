//
// Created by rika on 16/11/2019.
//

#include <QTimer>
#include "Analyser.h"
#include "../log/simpleQtLogger.h"
#include "../Exceptions.h"

#ifdef Q_OS_WASM
#   include <emscripten/html5.h>
#endif

using namespace Eigen;

Analyser::Analyser(AudioInterface * audioInterface)
    : audioInterface(audioInterface),
      frameLength(25),
      frameSpace(10),
      windowSpan(1), 
      frameCount(0),
      nsamples(0),
      doAnalyse(true),
      nfft(1),
      maximumFrequency(3700),
      lpFailed(true),
      nbNewFrames(0),
      running(false)
{
    _initResampler();

    loadSettings();

    setInputDevice(nullptr);
}

Analyser::~Analyser() {
    if (isAnalysing()) {
        stopThread();
    }
    saveSettings();
    ma_resampler_uninit(&resampler);
}

void Analyser::startThread() {
    running.store(true);
#ifndef Q_OS_WASM
    thread = std::thread(&Analyser::mainLoop, this);
#else
    emscripten_set_timeout_loop(
            [](double, void *p) -> EM_BOOL {
                auto that = static_cast<Analyser *>(p);
                that->update();
                return that->running.load();
            }, frameSpace.count(), this);
#endif
}

void Analyser::stopThread() {
    running.store(false);
#ifndef Q_OS_WASM
    thread.join();
#endif
}

void Analyser::toggle(bool running) {
    std::lock_guard<std::mutex> lock(mutex);
    doAnalyse = running;
}

bool Analyser::isAnalysing() {
    std::lock_guard<std::mutex> lock(mutex);
    return doAnalyse;
}

void Analyser::setInputDevice(const ma_device_id * id) {
    std::lock_guard<std::mutex> guard(audioLock);
    audioInterface->closeStream();
    audioInterface->openInputDevice(id);
    fs = audioInterface->getRecordSampleRate();
    audioInterface->startStream();
}

void Analyser::setOutputDevice(const ma_device_id * id) {
    std::lock_guard<std::mutex> guard(audioLock);
    audioInterface->closeStream();
    audioInterface->openOutputDevice(id);
    fs = audioInterface->getRecordSampleRate();
    audioInterface->startStream();
}

double Analyser::getSampleRate() {
    return audioInterface->getRecordSampleRate();
}

void Analyser::setFftSize(int _nfft, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    nfft = _nfft;
    LS_INFO("Set FFT size to " << nfft);
    
    _updateCaptureDuration();

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int Analyser::getFftSize() {
    std::lock_guard<std::mutex> lock(paramLock);
    return nfft;
}

void Analyser::setLinearPredictionOrder(int _lpOrder, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    lpOrder = std::clamp(_lpOrder, 4, 30);
    LS_INFO("Set LP order to " << lpOrder);

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int Analyser::getLinearPredictionOrder() {
    std::lock_guard<std::mutex> lock(paramLock);
    return lpOrder;
}

void Analyser::setCepstralOrder(int _cepOrder, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    
    cepOrder = std::clamp(_cepOrder, 7, 25);
    LS_INFO("Set LPCC order to " << cepOrder);

    _initEkfState(); 

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

int Analyser::getCepstralOrder() {
    std::lock_guard<std::mutex> lock(paramLock);
    return cepOrder;
}

void Analyser::setMaximumFrequency(double _maximumFrequency, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    maximumFrequency = std::clamp(_maximumFrequency, 2500.0, 7000.0);
   
    LS_INFO("Set maximum frequency to " << maximumFrequency);

    if (ma_resampler_set_rate(&resampler, audioInterface->getRecordSampleRate(), 2 * maximumFrequency) != MA_SUCCESS) {
        LS_FATAL("Unable to change analysis resampler output rate");
        throw AudioException("Unable to change analysis resampler output rate");
    }

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

double Analyser::getMaximumFrequency() {
    std::lock_guard<std::mutex> lock(paramLock);
    return maximumFrequency;
}

void Analyser::setFrameLength(const std::chrono::duration<double, std::milli> & _frameLength, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    frameLength = _frameLength;
    LS_INFO("Set frame length to " << frameLength.count() << " ms");
    
    _updateCaptureDuration();

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

const std::chrono::duration<double, std::milli> & Analyser::getFrameLength() {
    std::lock_guard<std::mutex> lock(paramLock);
    return frameLength;
}

void Analyser::setFrameSpace(const std::chrono::duration<double, std::milli> & _frameSpace, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    frameSpace = _frameSpace;
    LS_INFO("Set frame space to " << frameSpace.count() << " ms");
    
    _updateFrameCount();

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

const std::chrono::duration<double, std::milli> & Analyser::getFrameSpace() {
    std::lock_guard<std::mutex> lock(paramLock);
    std::lock_guard<std::mutex> lock2(mutex);
    return frameSpace;
}

void Analyser::setWindowSpan(const std::chrono::duration<double> & _windowSpan, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    windowSpan = _windowSpan;
    LS_INFO("Set window span to " << windowSpan.count() << " s");
    
    _updateFrameCount();

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

const std::chrono::duration<double> & Analyser::getWindowSpan() {
    std::lock_guard<std::mutex> lock(paramLock);
    std::lock_guard<std::mutex> lock2(mutex);
    return windowSpan;
}

void Analyser::setPitchAlgorithm(enum PitchAlg _pitchAlg, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    pitchAlg = _pitchAlg;

    switch (pitchAlg) {
        case Wavelet:
            L_INFO("Set pitch algorithm to DynamicWavelet");
            break;
        case McLeod:
            L_INFO("Set pitch algorithm to McLeod");
            break;
        case YIN:
            L_INFO("Set pitch algorithm to Yin");
            break;
        case AMDF:
            L_INFO("Set pitch algorithm to AMDF");
            break;
        /*case CREPE:
            L_INFO("Set pitch algorithm to CREPE");
            break;*/
    }

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

PitchAlg Analyser::getPitchAlgorithm() {
    std::lock_guard<std::mutex> lock(paramLock);
    return pitchAlg;
}

void Analyser::setFormantMethod(enum FormantMethod _method, bool save) {
    std::lock_guard<std::mutex> lock(paramLock);
    formantMethod = _method;

    switch (formantMethod) {
        case LP:
            L_INFO("Set formant algorithm to Linear Prediction");
            break;
        case KARMA:
            L_INFO("Set formant algorithm to KARMA");
            break;
        case DeepFormants:
            L_INFO("Set formant algorithm to DeepFormants");
            break;
    }

#ifdef Q_OS_WASM
    if (save)
        saveSettings();
#endif
}

FormantMethod Analyser::getFormantMethod() {
    std::lock_guard<std::mutex> lock(paramLock);
    return formantMethod;
}

int Analyser::getFrameCount() {
    std::lock_guard<std::mutex> lock(paramLock);
    std::lock_guard<std::mutex> lock2(mutex);

    return frameCount;
}

const SpecFrame & Analyser::getSpectrumFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return spectra.at(iframe);
}

const Formant::Frame & Analyser::getFormantFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return smoothedFormants.at(iframe);
}

double Analyser::getPitchFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return smoothedPitch.at(iframe);
}

double Analyser::getOqFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return oqTrack.at(iframe);
}

void Analyser::_updateFrameCount() {
    std::lock_guard<std::mutex> lock(mutex);

    const int newFrameCount = (1000 * windowSpan.count()) / frameSpace.count();

    static const Formant::Frame defaultFrame = {
        .nFormants = 5,
        .formant = {{550, 60}, {1650, 60}, {2750, 60}, {3850, 60}, {4950, 60}},
        .intensity = 1.0,
    };

    static const SpecFrame defaultSpec = {
        .fs = 16000,
        .nfft = 512,
        .spec = ArrayXd::Zero(512),
    };

    lpcSpectrum = defaultSpec;
    speechSignal.setZero(frameSamples);
    sourceSignal.setZero(frameSamples);

    if (frameCount < newFrameCount) {
        int diff = newFrameCount - frameCount;

        pitchTrack.insert(pitchTrack.begin(), diff, 0.0);
        formantTrack.insert(formantTrack.begin(), diff, defaultFrame);

        spectra.insert(spectra.begin(), diff, defaultSpec);
        smoothedPitch.insert(smoothedPitch.begin(), diff, 0.0);
        smoothedFormants.insert(smoothedFormants.begin(), diff, defaultFrame);
        oqTrack.insert(oqTrack.begin(), diff, 0.0);
    }
    else if (frameCount > newFrameCount) {
        int diff = frameCount - newFrameCount;

        pitchTrack.erase(pitchTrack.begin(), pitchTrack.begin() + diff);
        formantTrack.erase(formantTrack.begin(), formantTrack.begin() + diff);
        
        spectra.erase(spectra.begin(), spectra.begin() + diff);
        smoothedPitch.erase(smoothedPitch.begin(), smoothedPitch.begin() + diff);
        smoothedFormants.erase(smoothedFormants.begin(), smoothedFormants.begin() + diff);
        oqTrack.erase(oqTrack.begin(), oqTrack.begin() + diff);
    }

    LS_INFO("Resized tracks from " << frameCount << " to " << newFrameCount);

    frameCount = newFrameCount;
}

void Analyser::_updateCaptureDuration()
{
    std::lock_guard<std::mutex> lock(audioLock);

    double fs = audioInterface->getRecordSampleRate();

    // Account for resampling.
    // 1024 for crepe pitch algorithm
    fftSamples = std::max(1024, nfft); //(fs * nfft) / refs;
    frameSamples = frameLength.count() / 1000.0 * fs;

    int nsamples = std::max(fftSamples, frameSamples);
    if (nsamples != this->nsamples) {
        audioInterface->setCaptureDuration(nsamples);
        this->nsamples = nsamples;

        LS_INFO("Set capture duration to " << nsamples << " samples (" << (1000.0 * nsamples / fs) << " ms)");
    }
}

void Analyser::_initEkfState()
{
    int numF = 3;

    VectorXd x0(2 * numF);
    x0.setZero();

    for (int k = 0; k < numF; ++k) {
        x0(k) = 550 + 600 * k;
        x0(numF + k) = 90 + 20 * k;
    }

    ekfState.cepOrder = this->cepOrder;

    EKF::init(ekfState, x0);
}

void Analyser::_initResampler()
{
    ma_resampler_config config = ma_resampler_config_init(
            ma_format_f32,
            1,
            audioInterface->getRecordSampleRate(),
            2 * maximumFrequency,
#ifndef Q_OS_WASM
            ma_resample_algorithm_speex);
    config.speex.quality = 10;
#else
            ma_resample_algorithm_linear);
#endif

    if (ma_resampler_init(&config, &resampler) != MA_SUCCESS) {
        LS_FATAL("Unable to initialise analysis resampler");
        throw AudioException("Unable to initialise analysis resampler");
    }
}

void Analyser::loadSettings(QSettings& settings)
{
    L_INFO("Loading analysis settings...");

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { loadSettings(settings); });
    }
    else {
        settings.beginGroup("analysis");

        setMaximumFrequency(settings.value("maxFreq", 4700.0).value<double>(), false);
        setFftSize(settings.value("fftSize", 512).value<int>(), false);
        setLinearPredictionOrder(settings.value("lpOrder", 12).value<int>(), false);
        setFrameLength(std::chrono::milliseconds(settings.value("frameLength", 35).value<int>()), false);

        setFrameSpace(std::chrono::milliseconds(settings.value("frameSpace",
#ifdef Q_OS_WASM
                            100
#else
                            15
#endif
        ).value<int>()), false);
        
        setWindowSpan(std::chrono::milliseconds(int(1000 * settings.value("windowSpan",
#ifdef Q_OS_WASM
                            15.0
#else
                            5.0
#endif
        ).value<double>())), false);

        setPitchAlgorithm((PitchAlg) settings.value("pitchAlg", static_cast<int>(Wavelet)).value<int>(), false);
        setFormantMethod((FormantMethod) settings.value("formantMethod", static_cast<int>(KARMA)).value<int>(), false);
        setCepstralOrder(settings.value("cepOrder", 15).value<int>(), false);

        settings.endGroup();
    }
}
 
void Analyser::saveSettings(QSettings& settings)
{
    L_INFO("Saving analysis settings...");

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { saveSettings(settings); });
    }
    else {
        settings.beginGroup("analysis");

        settings.setValue("maxFreq", maximumFrequency);
        settings.setValue("fftSize", nfft);
        settings.setValue("lpOrder", lpOrder);
        settings.setValue("cepOrder", cepOrder);
        settings.setValue("frameLength", frameLength.count());
        settings.setValue("frameSpace", frameSpace.count());
        settings.setValue("windowSpan", windowSpan.count());
        settings.setValue("pitchAlg", static_cast<int>(pitchAlg));
        settings.setValue("formantMethod", static_cast<int>(formantMethod));

        settings.endGroup();
    }
}
