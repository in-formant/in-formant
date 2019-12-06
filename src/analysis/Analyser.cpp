//
// Created by rika on 16/11/2019.
//

#include "Analyser.h"

using namespace Eigen;

static const Formant::Frame defaultFrame = {
        .nFormants = 0,
        .formant = {},
        .intensity = 1.0,
};

Analyser::Analyser()
    : doAnalyse(true),
      lpOrder(10),
      maximumFrequency(5000.0),
      frameSpace(10.0),
      windowSpan(15.0),
      running(false)
{
    fs = audioCapture.getSampleRate();

    frameCount = 0;
    _updateFrameCount();

    // Initialize the audio frames to zero.
    x.setZero(CAPTURE_SAMPLE_COUNT(fs));
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
    doAnalyse = !doAnalyse;
}

void Analyser::setLinearPredictionOrder(int _lpOrder) {
    lpOrder = std::clamp(_lpOrder, 5, 22);
}

int Analyser::getLinearPredictionOrder() const {
    return lpOrder;
}

void Analyser::setMaximumFrequency(double _maximumFrequency) {
    maximumFrequency = std::clamp(_maximumFrequency, 2500.0, 7000.0);
}

double Analyser::getMaximumFrequency() const {
    return maximumFrequency;
}

void Analyser::setFrameSpace(const std::chrono::duration<double, std::milli> & _frameSpace) {
    frameSpace = _frameSpace;
    _updateFrameCount();
}

const std::chrono::duration<double, std::milli> & Analyser::getFrameSpace() const {
    return frameSpace;
}

void Analyser::setWindowSpan(const std::chrono::duration<double> & _windowSpan) {
    windowSpan = _windowSpan;
    _updateFrameCount();
}

const std::chrono::duration<double> & Analyser::getWindowSpan() const {
    return windowSpan;
}

int Analyser::getFrameCount() {
    std::lock_guard<std::mutex> lock(mutex);

    return frameCount;
}

const Formant::Frame & Analyser::getFormantFrame(int iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    iframe = std::clamp(iframe, 0, frameCount - 1);
    return formantTrack[iframe];
}

double Analyser::getPitchFrame(int iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    iframe = std::clamp(iframe, 0, frameCount - 1);
    return pitchTrack[iframe];
}

bool Analyser::isFrameVoiced(int iframe) {
    return getPitchFrame(iframe) > 0.0;
}

void Analyser::_updateFrameCount() {
    std::lock_guard<std::mutex> lock(mutex);

    int newFrameCount = (1000 * windowSpan.count()) / frameSpace.count();

    if (newFrameCount < frameCount) {
        int diff = frameCount - newFrameCount;
        pitchTrack.erase(pitchTrack.begin(), pitchTrack.begin() + diff);
        formantTrack.erase(formantTrack.begin(), formantTrack.begin() + diff);
    }
    else if (newFrameCount > frameCount) {
        int diff = newFrameCount - frameCount;
        pitchTrack.insert(pitchTrack.begin(), diff, 0.0);
        formantTrack.insert(formantTrack.begin(), diff, defaultFrame);
    }

    frameCount = newFrameCount;
}