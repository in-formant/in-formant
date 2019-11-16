//
// Created by rika on 16/11/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSER_H
#define SPEECH_ANALYSIS_ANALYSER_H

#include <Eigen/Core>
#include <deque>
#include <thread>
#include "../audio/AudioCapture.h"
#include "../lib/Formant/Formant.h"

constexpr double analysisUpdatesPerSecond = 1000.0 / 10.0; // This is to get a new frame every ~10ms.
constexpr int analysisFrameCount = 2000;
constexpr int analysisAudioFrameCount = 4;

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
    [[nodiscard]] const Formant::Frame & getFormantFrame(int iframe, bool raw);
    [[nodiscard]] double getPitchFrame(int iframe);
    [[nodiscard]] bool isFrameVoiced(int iframe);

private:
    void mainLoop();
    void update();

    void analysePitch();
    void resampleAudio();
    void analyseLp();
    void analyseFormants();

    AudioCapture audioCapture;

    // Parameters.
    bool doAnalyse;
    double maximumFrequency;
    int lpOrder;

    // Intermediate variables for analysis.
    std::array<Eigen::ArrayXd, analysisAudioFrameCount> audioFrames;
    Eigen::ArrayXd x;
    double fs;
    LPC::Frame lpcFrame;

    Formant::Frames rawFormantTrack;
    Formant::Frames formantTrack;
    std::deque<double> pitchTrack;

    // Thread-related members
    std::thread thread;
    std::atomic<bool> running;
    std::mutex mutex;
};


#endif //SPEECH_ANALYSIS_ANALYSER_H
