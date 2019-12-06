//
// Created by rika on 16/11/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSER_H
#define SPEECH_ANALYSIS_ANALYSER_H

#include <QColor>
#include <Eigen/Core>
#include <deque>
#include <thread>
#include <memory>
#include "../audio/AudioCapture.h"
#include "../lib/Formant/Formant.h"

constexpr QRgb formantColors[9] = {
        0xFFA700,
        0xFF57D9,
        0x7FFF00,
        0x57C8C8,
        0xC8A7FF,
        0x00A79C,
        0xFFFFFF, // unused
        0xFFFFFF,
        0xFFFFFF,
};

class Analyser {
public:
    Analyser();

    void startThread();
    void stopThread();

    void toggle();

    void setLinearPredictionOrder(int);
    void setMaximumFrequency(double);
    void setFrameSpace(const std::chrono::duration<double, std::milli> & frameSpace);
    void setWindowSpan(const std::chrono::duration<double> & windowSpan);

    [[nodiscard]] int getLinearPredictionOrder() const;
    [[nodiscard]] double getMaximumFrequency() const;

    [[nodiscard]] const std::chrono::duration<double, std::milli> & getFrameSpace() const;
    [[nodiscard]] const std::chrono::duration<double> & getWindowSpan() const;

    [[nodiscard]] int getFrameCount();
    [[nodiscard]] const Formant::Frame & getFormantFrame(int iframe);
    [[nodiscard]] double getPitchFrame(int iframe);
    [[nodiscard]] bool isFrameVoiced(int iframe);

private:
    void _updateFrameCount();

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
    std::chrono::duration<double, std::milli> frameSpace;
    std::chrono::duration<double> windowSpan;
    int frameCount;

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
