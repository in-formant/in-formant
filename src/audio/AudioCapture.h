//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIOCAPTURE_H
#define SPEECH_ANALYSIS_AUDIOCAPTURE_H

#include <soundio/soundio.h>
#include <Eigen/Core>
#include <string>
#include <memory>
#include <vector>

#define CAPTURE_DURATION 60
#define CAPTURE_SAMPLE_COUNT(sampleRate) ((CAPTURE_DURATION * sampleRate) / 1000)

#define BUFFER_SAMPLE_COUNT(sampleRate) ((5 * CAPTURE_DURATION * sampleRate) / 1000)

struct RecordContext {
    struct SoundIoRingBuffer * buffer;
};

class AudioCapture {
public:
    AudioCapture();
    ~AudioCapture();

    void selectCaptureDevice(int index = -1);
    void openInputStream();

    static void readCallback(struct SoundIoInStream * instream, int frame_count_min, int frame_count_max);
    static void overflowCallback(struct SoundIoInStream * instream);

    [[nodiscard]]
    int getSampleRate() const noexcept;

    [[nodiscard]]
    const std::vector<std::string> & getAvailableCaptureDevices() const noexcept;

    void readBlock(Eigen::ArrayXd & capture) const noexcept;

private:
    struct SoundIo * soundio;

    struct SoundIoDevice * inputDevice;
    int selectedSampleRate;
    enum SoundIoFormat selectedFormat;

    struct SoundIoInStream * inputStream;

    // Ring buffer
    struct RecordContext audioContext;

};

#endif //SPEECH_ANALYSIS_AUDIOCAPTURE_H
