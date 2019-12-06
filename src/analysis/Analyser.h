//
// Created by rika on 16/11/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSER_H
#define SPEECH_ANALYSIS_ANALYSER_H

#include <Eigen/Core>
#include <deque>
#include <thread>
#include <memory>
#include "../audio/AudioCapture.h"
#include "../lib/Formant/Formant.h"

constexpr double analysisUpdatesPerSecond = 1000.0 / 10.0; // This is to get a new frame every ~15ms.
constexpr int analysisFrameCount = 1500;

class Analyser {
public:
    Analyser();

    void startThread();
    void stopThread();

    void toggle();

    void setLinearPredictionOrder(int);
    void setMaximumFrequency(double);

    [[nodiscard]] int getLinearPredictionOrder() const;
    [[nodiscard]] double getMaximumFrequency() const;

    [[nodiscard]] const Formant::Frame & getFormantFrame(int iframe);
    [[nodiscard]] double getPitchFrame(int iframe);
    [[nodiscard]] bool isFrameVoiced(int iframe);

private:
    void mainLoop();
    void update();

    void analysePitch();
    void resampleAudio();
    void preEmphGauss();
    void analyseLp();
    void analyseFormantLp();
    void analyseFormantDeep();

    AudioCapture audioCapture;

    // Parameters.
    bool doAnalyse;
    double maximumFrequency;
    int lpOrder;

    // Intermediate variables for analysis.
    Eigen::ArrayXd x;
    double fs;
    LPC::Frame lpcFrame;

    Formant::Frames formantTrack;
    std::deque<double> pitchTrack;

    Formant::Frame lastFormantFrame;
    double lastPitchFrame;

    // Thread-related members
    std::thread thread;
    std::atomic<bool> running;
    std::mutex mutex;
};


#endif //SPEECH_ANALYSIS_ANALYSER_H
