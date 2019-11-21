//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AudioCapture.h"
#include "../Exceptions.h"

static constexpr std::array<int, 2> preferredLayouts = {
    1, 2
};

static constexpr std::array<PaSampleFormat, 5> preferredFormats = {
    paFloat32,
    paInt32,
    paInt16,
    paInt8,
    paUInt8,
};

static constexpr std::array<double, 5> preferredSampleRates = {
    22050,
    16000,
    32000,
    44100,
    48000,
};

template<typename T>
static void convertEndian(T array[], int length);

AudioCapture::AudioCapture() {

    err = Pa_Initialize();
    if (err != paNoError) {
        throw PaException("Unable to initialise", err);
    }

    // Open and start input stream.
    openInputStream();
    startInputStream();

}

AudioCapture::~AudioCapture() {

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cerr << "Unable to close stream: " << Pa_GetErrorText(err) << std::endl;
    }

    err = Pa_Terminate();
    if (err != paNoError) {
        std::cerr << "Unable to terminate: " << Pa_GetErrorText(err) << std::endl;
    }

}

void AudioCapture::openInputStream() {

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        throw PaException("Unable to open stream", "no default input device");
    }
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    for (int channelCount : preferredLayouts) {
        inputParameters.channelCount = channelCount;

        for (PaSampleFormat format : preferredFormats) {
            inputParameters.sampleFormat = format;

            for (double preferredSampleRate : preferredSampleRates) {

                err = Pa_IsFormatSupported(&inputParameters, nullptr, preferredSampleRate);
                if (err == paFormatIsSupported) {
                    sampleRate = preferredSampleRate;
                    goto found;
                }
            }
        }
    }

found:

    audioContext.format = inputParameters.sampleFormat;
    audioContext.buffer.setCapacity(BUFFER_SAMPLE_COUNT(sampleRate));

    err = Pa_OpenStream(
            &stream,
            &inputParameters,
            nullptr,
            sampleRate,
            paFramesPerBufferUnspecified,
            paClipOff,
            &readCallback,
            &audioContext);

    if (err != paNoError) {
        throw PaException("Unable to open stream", err);
    }
}

void AudioCapture::startInputStream() {

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        throw PaException("Unable to start stream", err);
    }

}

int AudioCapture::getSampleRate() const noexcept {
    return sampleRate;
}

void AudioCapture::readBlock(Eigen::ArrayXd & capture) noexcept {
    capture.conservativeResize(CAPTURE_SAMPLE_COUNT(sampleRate));
    audioContext.buffer.readFrom(capture);
}

template<typename T>
void convertEndian(T array[], int length) {

    static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    for (int k = 0; k < length; ++k) {
        source.u = array[k];

        for (size_t ib = 0; ib < sizeof(T); ++ib) {
            dest.u8[ib] = source.u8[sizeof(T) - ib - 1];
        }

        array[k] = dest.u;
    }

}
