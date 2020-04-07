//
// Created by rika on 16/11/2019.
//
//
#ifndef SPEECH_ANALYSIS_ANALYSER_H
#define SPEECH_ANALYSIS_ANALYSER_H

#include "../audio/miniaudio.h"
#include <QColor>
#include <Eigen/Core>
#include <deque>
#include <thread>
#include <memory>
#include <map>
#include "../audio/AudioCapture.h"
#include "../audio/AudioDevices.h"
#include "../lib/Formant/Formant.h"
#include "../lib/Formant/EKF/EKF.h"

struct SpecFrame {
    double fs;
    int nfft;
    Eigen::ArrayXd spec;
};

enum PitchAlg {
    Wavelet = 0,
    McLeod,
    YIN,
    AMDF,
};

enum FormantMethod {
    LP = 0,
    KARMA,
};

class Analyser {
public:
    Analyser(ma_context * ctx);
    ~Analyser();

    void startThread();
    void stopThread();

    void toggle();
    
    void setInputDevice(const ma_device_id * id);
    void setOutputDevice(const ma_device_id * id);

    void setFftSize(int);
    void setLinearPredictionOrder(int);
    void setMaximumFrequency(double);
    void setCepstralOrder(int);
    void setFrameLength(const std::chrono::duration<double, std::milli> & frameLength);
    void setFrameSpace(const std::chrono::duration<double, std::milli> & frameSpace);
    void setWindowSpan(const std::chrono::duration<double> & windowSpan);
    void setPitchAlgorithm(enum PitchAlg);
    void setFormantMethod(enum FormantMethod);

    [[nodiscard]] bool isAnalysing();
    [[nodiscard]] int getFftSize();
    [[nodiscard]] int getLinearPredictionOrder();
    [[nodiscard]] double getMaximumFrequency();
    [[nodiscard]] int getCepstralOrder();
    [[nodiscard]] const std::chrono::duration<double, std::milli> & getFrameLength();
    [[nodiscard]] const std::chrono::duration<double, std::milli> & getFrameSpace();
    [[nodiscard]] const std::chrono::duration<double> & getWindowSpan();
    [[nodiscard]] PitchAlg getPitchAlgorithm();
    [[nodiscard]] FormantMethod getFormantMethod();

    [[nodiscard]] int getFrameCount();

    [[nodiscard]] const SpecFrame & getSpectrumFrame(int iframe);
    [[nodiscard]] const Formant::Frame & getFormantFrame(int iframe);
    [[nodiscard]] double getPitchFrame(int iframe);

    [[nodiscard]] const SpecFrame & getLastSpectrumFrame();
    [[nodiscard]] const Formant::Frame & getLastFormantFrame();
    [[nodiscard]] double getLastPitchFrame();

private:
    void loadSettings();
    void saveSettings();

    void _updateFrameCount();
    void _updateCaptureDuration();
    void _initEkfState();

    void mainLoop();
    void update();
    void applyWindow();
    void analyseSpectrum();
    void analysePitch();
    void resampleAudio(double newFs);
    void applyPreEmphasis();
    void analyseLp();
    void analyseFormant();
    void analyseFormantEkf();
    void trackFormants();
    void applySmoothingFilters();

    std::mutex audioLock;
    AudioCapture * audioCapture;

    // Parameters.
    std::mutex paramLock;

    std::chrono::duration<double, std::milli> frameLength;
    std::chrono::duration<double, std::milli> frameSpace;
    std::chrono::duration<double> windowSpan;
    int frameCount;

    int fftSamples;
    int frameSamples;
    int nsamples;

    bool doAnalyse;
    int nfft;
    double maximumFrequency;
    int lpOrder;
    int cepOrder;

    FormantMethod formantMethod;
    PitchAlg pitchAlg;

    // Intermediate variables for analysis.
    Eigen::ArrayXd x, x_fft;
    double fs;
    LPC::Frame lpcFrame;
    EKF::State ekfState;

    Formant::Frames formantTrack;
    std::deque<double> pitchTrack;

    std::deque<SpecFrame> spectra;
    Formant::Frames smoothedFormants;
    std::deque<double> smoothedPitch;

    SpecFrame lastSpectrumFrame;
    Formant::Frame lastFormantFrame;
    double lastPitchFrame;

    bool lpFailed;
    std::map<int, int> nbNewFrames;

    // Thread-related members
    std::thread thread;
    std::atomic<bool> running;
    std::mutex mutex;

public:

    template<typename Func1, typename Func2, typename Func3>
    void callIfNewFrames(int nb, Func1 fn1, Func2 fn2, Func3 fn3)
    {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (nbNewFrames.find(nb) == nbNewFrames.end()) {
            nbNewFrames[nb] = 0;
        }

        if (nbNewFrames[nb] > 0) {
            fn1(frameCount, maximumFrequency, smoothedPitch, smoothedFormants);
            fn2(frameCount, nbNewFrames[nb], maximumFrequency, spectra.cend() - 1 - nbNewFrames[nb], spectra.cend());
            nbNewFrames[nb] = 0;
        }

        fn3(frameCount, maximumFrequency);
    }

};


#endif //SPEECH_ANALYSIS_ANALYSER_H
