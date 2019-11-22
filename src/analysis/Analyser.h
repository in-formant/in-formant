//
// Created by rika on 16/11/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSER_H
#define SPEECH_ANALYSIS_ANALYSER_H

#include <Eigen/Core>
#include <deque>
#include <thread>
#include <memory>
#include <pitch_detection.h>
#include "../audio/AudioCapture.h"
#include "../lib/Formant/Formant.h"

constexpr double analysisUpdatesPerSecond = 1000.0 / 15.0; // This is to get a new frame every ~15ms.
constexpr int analysisFrameCount = 2000;
constexpr int analysisPitchFrameCount = (int) analysisUpdatesPerSecond / 2; // This is to refine pitch roughly twice per second.
constexpr int analysisPitchFrameOverlap = 5;
constexpr int analysisCleanupFftTime = 10;

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

    void normalizeFrame();
    void analysePitch();
    void resampleAudio();
    void analyseLp();
    void analyseFormants();
    void refinePitch();

    AudioCapture audioCapture;

    // Parameters.
    bool doAnalyse;
    double maximumFrequency;
    int lpOrder;

    // Intermediate variables for analysis.
    std::array<Eigen::ArrayXd, analysisPitchFrameCount> audioFrames;
    int frameCount, refineCount;
    std::shared_ptr<pitch_alloc::Yin<float>> mpm;

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
