//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIOCAPTURE_H
#define SPEECH_ANALYSIS_AUDIOCAPTURE_H

#include <portaudio.h>
#include <Eigen/Core>
#include <string>
#include <memory>
#include <vector>
#include "RingBuffer.h"

#define CAPTURE_DURATION 60.0
#define CAPTURE_SAMPLE_COUNT(sampleRate) ((CAPTURE_DURATION * sampleRate) / 1000)

#define BUFFER_SAMPLE_COUNT(sampleRate) ((2 * CAPTURE_DURATION * sampleRate) / 1000)

struct RecordContext {
    RingBuffer buffer;
    PaSampleFormat format;
};

class AudioCapture {
public:
    AudioCapture();
    ~AudioCapture();

    void openInputStream();
    void startInputStream();

    static int readCallback(const void * input, void * output,
                     unsigned long frameCount,
                     const PaStreamCallbackTimeInfo * timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void * userData);

    [[nodiscard]]
    int getSampleRate() const noexcept;

    void readBlock(Eigen::ArrayXd & capture) noexcept;

private:
    PaError err;
    PaStream * stream;

    PaStreamParameters inputParameters;
    double sampleRate;

    // Ring buffer
    struct RecordContext audioContext;

};

#endif //SPEECH_ANALYSIS_AUDIOCAPTURE_H
