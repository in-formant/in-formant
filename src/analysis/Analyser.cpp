//
// Created by rika on 16/11/2019.
//

#include "Analyser.h"

using namespace Eigen;

static const Formant::Frame defaultFrame = {
        .nFormants = 5,
        .formant = {{}, {}, {}, {}, {}},
        .intensity = 1.0,
};

Analyser::Analyser()
    : doAnalyse(true),
      lpOrder(10),
      maximumFrequency(5000.0),
      running(false),
      frameCount(0),
      refineCount(0)
{
    fs = audioCapture.getSampleRate();

    rawFormantTrack.resize(analysisFrameCount, defaultFrame);
    formantTrack.resize(analysisFrameCount, defaultFrame);
    pitchTrack.resize(analysisFrameCount, 0.0);

    // Initialize the audio frames to zero.
    x.setZero(CAPTURE_SAMPLE_COUNT(fs));
    for (int i = 0; i < analysisPitchFrameCount; ++i) {
        audioFrames[i].setZero(CAPTURE_SAMPLE_COUNT(fs));
    }
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
    maximumFrequency = std::clamp(_maximumFrequency, 3000.0, 6000.0);
}

double Analyser::getMaximumFrequency() const {
    return maximumFrequency;
}

const Formant::Frame & Analyser::getFormantFrame(int iframe, bool raw) {
    std::lock_guard<std::mutex> lock(mutex);

    iframe = std::clamp(iframe, 0, analysisFrameCount - 1);
    return raw ? rawFormantTrack[iframe] : formantTrack[iframe];
}

double Analyser::getPitchFrame(int iframe) {
    std::lock_guard<std::mutex> lock(mutex);

    iframe = std::clamp(iframe, 0, analysisFrameCount - 1);
    return pitchTrack[iframe];
}

bool Analyser::isFrameVoiced(int iframe) {
    return getPitchFrame(iframe) > 0.0;
}