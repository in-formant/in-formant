//
// Created by clo on 12/09/2019.
//

#include <QSettings>
#include <iostream>
#include "AudioInterface.h"
#include "../Exceptions.h"
#include "../log/simpleQtLogger.h"

AudioInterface::AudioInterface(ma_context * maCtx, SineWave * sineWave, NoiseFilter * noiseFilter)
    : maCtx(maCtx),
      deviceCaptureInit(false),
      devicePlaybackInit(false), 
      sampleRate(16000)
{
    L_INFO("Initialising audio capture buffer...");

    recordContext.sampleRate = sampleRate;
    playbackContext.sineWave = sineWave;
    playbackContext.noiseFilter = noiseFilter;
}

AudioInterface::~AudioInterface()
{
    closeStream();
}

void AudioInterface::openInputDevice(const ma_device_id * id)
{
    if (deviceCaptureInit) {
        return;
    }

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
    if (deviceCaptureInit) {
        return;
    }

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

void AudioInterface::openPlaybackDevice()
{

    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.pDeviceID = nullptr;
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.sampleRate = 96000; // This is needed to have a precise enough time resolution.
    deviceConfig.dataCallback = playCallback;
    deviceConfig.pUserData = &playbackContext;

    L_INFO("Opening output device for sine wave...");

    if (ma_device_init(maCtx, &deviceConfig, &devicePlayback) != MA_SUCCESS) {
        L_FATAL("Failed to open output device for sine wave...");
        throw AudioException("Failed to initialise miniaudio device");
    }

    playbackContext.numChannels = devicePlayback.playback.channels;
    playbackContext.sineWave->initWaveform(devicePlayback.sampleRate, playbackContext.numChannels);
    playbackContext.noiseFilter->initOutput(devicePlayback.sampleRate, playbackContext.numChannels);

    devicePlaybackInit = true;
}

void AudioInterface::startStream() {

    if (deviceCaptureInit) {

        L_INFO("Starting capture audio device...");

        if (ma_device_start(&deviceCapture) != MA_SUCCESS) {
            L_FATAL("Failed to start capture audio device...");
            throw AudioException("Failed to start miniaudio device");
        }
    }

    if (devicePlaybackInit) {

        L_INFO("Starting playback audio device...");

        if (ma_device_start(&devicePlayback) != MA_SUCCESS) {
            L_FATAL("Failed to start playback audio device...");
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

int AudioInterface::getRecordSampleRate() const noexcept {
    return sampleRate;
}

int AudioInterface::getPlaybackSampleRate() const noexcept {
    return devicePlayback.sampleRate;
}

void AudioInterface::readBlock(Eigen::ArrayXd & capture) noexcept {
    recordContext.buffer.readFrom(capture);
}

NoiseFilter * AudioInterface::getNoiseFilter() {
    return playbackContext.noiseFilter;
}

