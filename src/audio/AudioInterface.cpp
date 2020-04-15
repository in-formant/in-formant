//
// Created by clo on 12/09/2019.
//

#include <QSettings>
#include <iostream>
#include "AudioInterface.h"
#include "../Exceptions.h"
#include "../log/simpleQtLogger.h"

AudioInterface::AudioInterface(ma_context * maCtx, SineWave * sineWave)
    : sampleRate(16000), maCtx(maCtx),
      deviceCaptureInit(false), devicePlaybackInit(false)
{
    L_INFO("Initialising audio capture buffer...");

    recordContext.sampleRate = sampleRate;
    playbackContext.sineWave = sineWave;
}

AudioInterface::~AudioInterface()
{
    closeStream();
}

void AudioInterface::openInputDevice(const ma_device_id * id)
{
    auto deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.pDeviceID = const_cast<ma_device_id *>(id);
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.periodSizeInFrames = 0;
    deviceConfig.periodSizeInMilliseconds = 25;
    deviceConfig.periods = 3;
    deviceConfig.noClip = true;
#ifndef Q_OS_ANDROID
    deviceConfig.resampling.algorithm = ma_resample_algorithm_speex;
    deviceConfig.resampling.speex.quality = 10;
#endif
    deviceConfig.dataCallback = recordCallback;
    deviceConfig.pUserData = &recordContext;
   
    L_INFO("Opening input device...");

    if (ma_device_init(maCtx, &deviceConfig, &deviceCapture) != MA_SUCCESS) {
        L_FATAL("Failed to open input device...");
        throw AudioException("Failed to initialise miniaudio device");
    }
    
    recordContext.numChannels = deviceCapture.capture.channels;
    
    deviceCaptureInit = true;
    
    openPlaybackDevice();
}

void AudioInterface::openOutputDevice(const ma_device_id * id)
{
    if (!ma_context_is_loopback_supported(maCtx)) {
        L_FATAL("Audio loopback devices are not supported.");
        throw AudioException("Failed to initialise loopback device: backend not supported");
    }

    auto deviceConfig = ma_device_config_init(ma_device_type_loopback);
    deviceConfig.capture.pDeviceID = const_cast<ma_device_id *>(id);
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.periodSizeInFrames = 0;
    deviceConfig.periodSizeInMilliseconds = 25;
    deviceConfig.periods = 3;
    deviceConfig.noClip = true;
#ifndef Q_OS_ANDROID
    deviceConfig.resampling.algorithm = ma_resample_algorithm_speex;
    deviceConfig.resampling.speex.quality = 10;
#endif
    deviceConfig.dataCallback = recordCallback;
    deviceConfig.pUserData = &recordContext;

    L_INFO("Opening output device as loopback...");

    if (ma_device_init(maCtx, &deviceConfig, &deviceCapture) != MA_SUCCESS) {
        L_FATAL("Failed to open output device...");
        throw AudioException("Failed to initialise miniaudio device");
    }

    recordContext.numChannels = deviceCapture.capture.channels;
    
    deviceCaptureInit = true; 

    openPlaybackDevice();
}

void AudioInterface::openPlaybackDevice() {

    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.pDeviceID = nullptr;
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.sampleRate = 96000;
    deviceConfig.dataCallback = playCallback;
    deviceConfig.pUserData = &playbackContext;

    L_INFO("Opening output device for sine wave...");

    if (ma_device_init(maCtx, &deviceConfig, &devicePlayback) != MA_SUCCESS) {
        L_FATAL("Failed to open output device for sine wave...");
        throw AudioException("Failed to initialise miniaudio device");
    }

    playbackContext.sineWave->initWaveform(devicePlayback.sampleRate, devicePlayback.playback.channels);

    devicePlaybackInit = true;
}

void AudioInterface::startStream() {

    if (devicePlaybackInit) {

        L_INFO("Starting playback audio device...");

        if (ma_device_start(&devicePlayback) != MA_SUCCESS) {
            L_FATAL("Failed to start playback audio device...");
            throw AudioException("Failed to start miniaudio device");
        }
    }

    if (deviceCaptureInit) {

        L_INFO("Starting capture audio device...");

        if (ma_device_start(&deviceCapture) != MA_SUCCESS) {
            L_FATAL("Failed to start capture audio device...");
            throw AudioException("Failed to start miniaudio device");
        }
    }

}

void AudioInterface::closeStream()
{
    if (deviceCaptureInit) {
        L_INFO("Closing capture audio device...");
        ma_device_uninit(&deviceCapture);
        deviceCaptureInit = false;
    }

    if (devicePlaybackInit) {
        L_INFO("Closing playback audio device...");
        ma_device_uninit(&devicePlayback);
        devicePlaybackInit = false;
    }
}

void AudioInterface::setCaptureDuration(int nsamples) {
    recordContext.buffer.setCapacity(nsamples);
}

int AudioInterface::getSampleRate() const noexcept {
    return sampleRate;
}

void AudioInterface::readBlock(Eigen::ArrayXd & capture) noexcept {
    recordContext.buffer.readFrom(capture);
}

