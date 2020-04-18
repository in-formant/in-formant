//
// Created by rika on 16/11/2019.
//
//
#ifndef SPEECH_ANALYSIS_ANALYSER_H
#define SPEECH_ANALYSIS_ANALYSER_H

#include "../audio/miniaudio.h"
#include <QColor>
#include <Eigen/Core>
#include <thread>
#include <memory>
#include "rpmalloc.h"
#include "../audio/AudioInterface.h"
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
    Analyser(AudioInterface * audioInterface);
    ~Analyser();

    void startThread();
    void stopThread();

    void toggle(bool running);
    
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

    [[nodiscard]] double getSampleRate();

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
    [[nodiscard]] double getOqFrame(int iframe);

private:
    void loadSettings();
    void saveSettings();

    void _updateFrameCount();
    void _updateCaptureDuration();
    void _initEkfState();
    void _initResampler();

    void mainLoop();
#ifdef Q_OS_WASM
public:
#endif
    void update();
private:
    void applyWindow();
    void analyseSpectrum();
    void analysePitch();
    void analyseOq();
    void resampleAudio();
    void applyPreEmphasis();
    void analyseLp();
    void analyseFormant();
    void analyseFormantEkf();
    void trackFormants();
    void applySmoothingFilters();

    std::mutex audioLock;

    AudioInterface * audioInterface;

    ma_resampler resampler;

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
    rpm::deque<double> pitchTrack;

    // Results
    SpecFrame lpcSpectrum;
    rpm::deque<SpecFrame> spectra;
    Formant::Frames smoothedFormants;
    rpm::deque<double> smoothedPitch;
    rpm::deque<double> oqTrack;

    SpecFrame lastSpectrumFrame;
    Formant::Frame lastFormantFrame;
    double lastPitchFrame;
    double lastOqFrame;

    bool lpFailed;
    int nbNewFrames;

    // Thread-related members
    std::thread thread;
    std::atomic<bool> running;
    std::mutex mutex;

public:

    template<typename Func1, typename Func2, typename Func3, typename Func4>
    void callIfNewFrames(Func1 fn1, Func2 fn2, Func3 fn3, Func4 fn4)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (nbNewFrames > 0) {
            fn1(frameCount, maximumFrequency, formantMethod, smoothedPitch, smoothedFormants);
            fn2(frameCount, nbNewFrames, maximumFrequency, spectra.cend() - 1 - nbNewFrames, spectra.cend());
            fn3(maximumFrequency, lpcSpectrum);
            nbNewFrames = 0;
        }

        fn4(frameCount, maximumFrequency);
    }

};


#endif //SPEECH_ANALYSIS_ANALYSER_H
