//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIOINTERFACE_H
#define SPEECH_ANALYSIS_AUDIOINTERFACE_H

#include "miniaudio.h"
#include <Eigen/Core>
#include "RingBuffer.h"
#include "SineWave.h"
#include "NoiseFilter.h"

#define CAPTURE_DURATION 50.0
#define CAPTURE_SAMPLE_COUNT(sampleRate) ((CAPTURE_DURATION * sampleRate) / 1000)

#define BUFFER_SAMPLE_COUNT(sampleRate) (CAPTURE_SAMPLE_COUNT(sampleRate))

struct RecordContext {
    RingBuffer buffer;
    double sampleRate;
    int numChannels;
};

struct PlaybackContext {
    SineWave * sineWave;
    NoiseFilter * noiseFilter;
    int numChannels;
};

class AudioInterface {
public:
    AudioInterface(ma_context * maCtx, SineWave * sineWave, NoiseFilter * noiseFilter);
    ~AudioInterface();

    void openInputDevice(const ma_device_id * id);
    void openOutputDevice(const ma_device_id * id);

    void openPlaybackDevice();

    void startStream();
    void closeStream();

    static void recordCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);
    static void playCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

    void setCaptureDuration(int nsamples);

    [[nodiscard]] int getRecordSampleRate() const noexcept;
    [[nodiscard]] int getPlaybackSampleRate() const noexcept;

    NoiseFilter * getNoiseFilter();

    void readBlock(Eigen::ArrayXd & capture) noexcept;

private:
    ma_context * maCtx;
    
    bool deviceCaptureInit;
    ma_device deviceCapture;
    
    bool devicePlaybackInit;
    ma_device devicePlayback;

    double sampleRate;

    // Record context
    struct RecordContext recordContext;

    // Playback context
    struct PlaybackContext playbackContext;

};

#endif //SPEECH_ANALYSIS_AUDIOINTERFACE_H
