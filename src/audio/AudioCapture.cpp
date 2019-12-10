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

AudioCapture::AudioCapture() {

    stream = nullptr;

    err = Pa_Initialize();
    if (err != paNoError) {
        throw PaException("Unable to initialise", err);
    }

}

AudioCapture::~AudioCapture() {

    closeStream();

    err = Pa_Terminate();
    if (err != paNoError) {
        std::cerr << "Unable to terminate: " << Pa_GetErrorText(err) << std::endl;
    }

}

void AudioCapture::openInputDevice(int id) {

    memset(&parameters, 0, sizeof(parameters));
    parameters.device = id;
    parameters.suggestedLatency = Pa_GetDeviceInfo(id)->defaultLowInputLatency;
    parameters.hostApiSpecificStreamInfo = nullptr;

    for (int channelCount : preferredLayouts) {
        parameters.channelCount = channelCount;

        for (PaSampleFormat format : preferredFormats) {
            parameters.sampleFormat = format;

            for (double preferredSampleRate : preferredSampleRates) {

                err = Pa_IsFormatSupported(&parameters, nullptr, preferredSampleRate);
                if (err == paFormatIsSupported) {
                    sampleRate = preferredSampleRate;
                    goto found;
                }
            }
        }
    }

found:

    audioContext.format = parameters.sampleFormat;
    audioContext.buffer.setCapacity(BUFFER_SAMPLE_COUNT(sampleRate));

    err = Pa_OpenStream(
            &stream,
            &parameters,
            nullptr,
            sampleRate,
            paFramesPerBufferUnspecified,
            paClipOff,
            &readCallback,
            &audioContext);

    if (err != paNoError) {
        throw PaException("Unable to open input stream", err);
    }
}

void AudioCapture::openOutputDevice(int id) {

    memset(&parameters, 0, sizeof(parameters));
    parameters.device = id;
    parameters.suggestedLatency = Pa_GetDeviceInfo(id)->defaultLowOutputLatency;
    parameters.hostApiSpecificStreamInfo = nullptr;

    for (int channelCount : preferredLayouts) {
        parameters.channelCount = channelCount;

        for (PaSampleFormat format : preferredFormats) {
            parameters.sampleFormat = format;

            for (double preferredSampleRate : preferredSampleRates) {

                err = Pa_IsFormatSupported(nullptr, &parameters, preferredSampleRate);
                if (err == paFormatIsSupported) {
                    sampleRate = preferredSampleRate;
                    goto found;
                }
            }
        }
    }

found:

    audioContext.format = parameters.sampleFormat;
    audioContext.buffer.setCapacity(BUFFER_SAMPLE_COUNT(sampleRate));

    err = Pa_OpenStream(
            &stream,
            nullptr,
            &parameters,
            sampleRate,
            paFramesPerBufferUnspecified,
            paClipOff,
            &readCallback,
            &audioContext);

    if (err != paNoError) {
        throw PaException("Unable to open output stream", err);
    }
}

void AudioCapture::startStream() {

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        throw PaException("Unable to start stream", err);
    }

}

void AudioCapture::closeStream() {

    if (stream != nullptr) {
        err = Pa_CloseStream(stream);
        if (err != paNoError) {
            std::cerr << "Unable to close stream: " << Pa_GetErrorText(err) << std::endl;
        }
    }
}


int AudioCapture::getSampleRate() const noexcept {
    return sampleRate;
}

void AudioCapture::readBlock(Eigen::ArrayXd & capture) noexcept {
    capture.conservativeResize(CAPTURE_SAMPLE_COUNT(sampleRate));
    audioContext.buffer.readFrom(capture);
}

