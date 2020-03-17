//
// Created by rika on 16/11/2019.
//

#include "Analyser.h"

using namespace Eigen;

static const Formant::Frame defaultFrame = {
    .nFormants = 5,
    .formant = {{550, 60}, {1650, 60}, {2750, 60}, {3850, 60}, {4950, 60}},
    .intensity = 1.0,
};

static const SpecFrame defaultSpec = {
    .fs = 0,
    .nfft = 512,
    .spec = ArrayXd::Zero(512),
};

Analyser::Analyser(ma_context * ctx)
    : audioCapture(ctx),
      doAnalyse(true),
      nfft(512),
      lpOrder(10),
      cepOrder(15),
      maximumFrequency(5000.0),
      frameSpace(15.0),
      windowSpan(5.0),
      running(false),
      lpFailed(true),
      nbNewFrames(0),
      formantMethod(KARMA),
      pitchAlg(Wavelet)
{
    frameCount = 0;
    _updateFrameCount();
    _initEkfState();

    // Initialize the audio frames to zero.
    x.setZero(512);

    setInputDevice(nullptr);
}

void Analyser::startThread() {
    running.store(true);
    thread = std::thread(&Analyser::mainLoop, this);
}

void Analyser::stopThread() {
    running.store(false);
    thread.join();
}

void Analyser::toggle() {
    std::lock_guard<std::mutex> lock(mutex);
    doAnalyse = !doAnalyse;
}

bool Analyser::isAnalysing() {
    std::lock_guard<std::mutex> lock(mutex);
    return doAnalyse;
}

void Analyser::setInputDevice(const ma_device_id * id) {
    std::lock_guard<std::mutex> guard(audioLock);
    audioCapture.closeStream();
    audioCapture.openInputDevice(id);
    fs = audioCapture.getSampleRate();
    x.setZero(CAPTURE_SAMPLE_COUNT(fs));
    audioCapture.startStream();
}

void Analyser::setOutputDevice(const ma_device_id * id) {
    std::lock_guard<std::mutex> guard(audioLock);
    audioCapture.closeStream();
    audioCapture.openOutputDevice(id);
    fs = audioCapture.getSampleRate();
    x.setZero(CAPTURE_SAMPLE_COUNT(fs));
    audioCapture.startStream();
}

void Analyser::setFftSize(int _nfft) {
    std::lock_guard<std::mutex> lock(mutex);
    nfft = _nfft;
}

int Analyser::getFftSize() {
    std::lock_guard<std::mutex> lock(mutex);
    return nfft;
}

void Analyser::setLinearPredictionOrder(int _lpOrder) {
    std::lock_guard<std::mutex> lock(mutex);
    lpOrder = std::clamp(_lpOrder, 5, 22);
}

int Analyser::getLinearPredictionOrder() {
    std::lock_guard<std::mutex> lock(mutex);
    return lpOrder;
}

void Analyser::setCepstralOrder(int _cepOrder) {
    std::lock_guard<std::mutex> lock(mutex);
    
    cepOrder = std::clamp(_cepOrder, 7, 25);
    _initEkfState();
}

int Analyser::getCepstralOrder() {
    std::lock_guard<std::mutex> lock(mutex);
    return cepOrder;
}

void Analyser::setMaximumFrequency(double _maximumFrequency) {
    std::lock_guard<std::mutex> lock(mutex);
    maximumFrequency = std::clamp(_maximumFrequency, 2500.0, 7000.0);
}

double Analyser::getMaximumFrequency() {
    std::lock_guard<std::mutex> lock(mutex);
    return maximumFrequency;
}

void Analyser::setFrameSpace(const std::chrono::duration<double, std::milli> & _frameSpace) {
    std::lock_guard<std::mutex> lock(mutex);
    frameSpace = _frameSpace;
    _updateFrameCount();
}

const std::chrono::duration<double, std::milli> & Analyser::getFrameSpace() {
    std::lock_guard<std::mutex> lock(mutex);
    return frameSpace;
}

void Analyser::setWindowSpan(const std::chrono::duration<double> & _windowSpan) {
    std::lock_guard<std::mutex> lock(mutex);
    windowSpan = _windowSpan;
    _updateFrameCount();
}

const std::chrono::duration<double> & Analyser::getWindowSpan() {
    std::lock_guard<std::mutex> lock(mutex);
    return windowSpan;
}

void Analyser::setPitchAlgorithm(enum PitchAlg _pitchAlg) {
    std::lock_guard<std::mutex> lock(mutex);
    pitchAlg = _pitchAlg;
}

void Analyser::setFormantMethod(enum FormantMethod _method) {
    std::lock_guard<std::mutex> lock(mutex);
    formantMethod = _method;
}

int Analyser::getFrameCount() {
    std::lock_guard<std::mutex> lock(mutex);

    return frameCount;
}

const SpecFrame & Analyser::getSpectrumFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return spectra.at(iframe);
}

const SpecFrame & Analyser::getLastSpectrumFrame() {
    std::lock_guard<std::mutex> lock(mutex);

    return spectra.back();
}

const Formant::Frame & Analyser::getFormantFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return smoothedFormants.at(iframe);
}

const Formant::Frame & Analyser::getLastFormantFrame() {
    std::lock_guard<std::mutex> lock(mutex);

    return formantTrack.back();
}

double Analyser::getPitchFrame(int _iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    int iframe = std::clamp(_iframe, 0, frameCount - 1);
    return smoothedPitch.at(iframe);
}

double Analyser::getLastPitchFrame() {
    std::lock_guard<std::mutex> lock(mutex);

    return pitchTrack.back();
}

void Analyser::_updateFrameCount() {
    const int newFrameCount = (1000 * windowSpan.count()) / frameSpace.count();

    if (frameCount < newFrameCount) {
        int diff = newFrameCount - frameCount;

        pitchTrack.insert(pitchTrack.begin(), diff, 0.0);
        formantTrack.insert(formantTrack.begin(), diff, defaultFrame);

        spectra.insert(spectra.begin(), diff, defaultSpec);
        smoothedPitch.insert(smoothedPitch.begin(), diff, 0.0);
        smoothedFormants.insert(smoothedFormants.begin(), diff, defaultFrame);
    }
    else if (frameCount > newFrameCount) {
        int diff = frameCount - newFrameCount;

        pitchTrack.erase(pitchTrack.begin(), pitchTrack.begin() + diff);
        formantTrack.erase(formantTrack.begin(), formantTrack.begin() + diff);
        
        spectra.erase(spectra.begin(), spectra.begin() + diff);
        smoothedPitch.erase(smoothedPitch.begin(), smoothedPitch.begin() + diff);
        smoothedFormants.erase(smoothedFormants.begin(), smoothedFormants.begin() + diff);
    }

    frameCount = newFrameCount;
}

void Analyser::_initEkfState()
{
    int numF = 4;

    VectorXd x0(2 * numF);
    x0.setZero();

    // Average over the last 20 frames.
    
    for (int i = 1; i <= 20; ++i) {
        VectorXd x(2 * numF);
        
        auto & frm = smoothedFormants[frameCount - i];

        for (int k = 0; k < std::min(frm.nFormants, numF); ++k) {
            x(k) = frm.formant[k].frequency;
            x(numF + k) = frm.formant[k].bandwidth;
        }
        
        x0 += x;
    }

    x0 /= 20.0;

    ekfState.cepOrder = this->cepOrder;

    EKF::init(ekfState, x0);
}
