//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIOCAPTURE_H
#define SPEECH_ANALYSIS_AUDIOCAPTURE_H

#include "miniaudio.h"
#include <Eigen/Core>
#include "RingBuffer.h"

#define CAPTURE_DURATION 50.0
#define CAPTURE_SAMPLE_COUNT(sampleRate) ((CAPTURE_DURATION * sampleRate) / 1000)

#define BUFFER_SAMPLE_COUNT(sampleRate) (CAPTURE_SAMPLE_COUNT(sampleRate))

struct RecordContext {
    RingBuffer buffer;
    double sampleRate;
};

class AudioCapture {
public:
    AudioCapture(ma_context * maCtx);
    ~AudioCapture();

    void openInputDevice(const ma_device_id * id);
    void openOutputDevice(const ma_device_id * id);
    void startStream();
    void closeStream();

    static void readCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

    void setCaptureDuration(int nsamples);

    [[nodiscard]] int getSampleRate() const noexcept;

    void readBlock(Eigen::ArrayXd & capture) noexcept;

private:
    ma_context * maCtx;
    bool deviceInit;
    ma_device device;

    double sampleRate;

    // Ring buffer
    struct RecordContext audioContext;

};

#endif //SPEECH_ANALYSIS_AUDIOCAPTURE_H
