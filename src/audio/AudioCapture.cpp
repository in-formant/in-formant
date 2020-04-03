//
// Created by clo on 12/09/2019.
//

#include <QSettings>
#include <iostream>
#include "AudioCapture.h"
#include "../Exceptions.h"
#include "../log/simpleQtLogger.h"

AudioCapture::AudioCapture(ma_context * maCtx)
    : sampleRate(16000), maCtx(maCtx), deviceInit(false)
{
    L_INFO("Initialising audio capture buffer...");

    audioContext.sampleRate = sampleRate;
}

AudioCapture::~AudioCapture()
{
    closeStream();
}

void AudioCapture::openInputDevice(const ma_device_id * id)
{
    auto deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.pDeviceID = const_cast<ma_device_id *>(id);
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.noClip = true;
    deviceConfig.dataCallback = readCallback;
    deviceConfig.pUserData = &audioContext;
   
    L_INFO("Opening input device...");

    if (ma_device_init(maCtx, &deviceConfig, &device) != MA_SUCCESS) {
        L_FATAL("Failed to open input device...");
        throw AudioException("Failed to initialise miniaudio device");
    }
    
    deviceInit = true;
}

void AudioCapture::openOutputDevice(const ma_device_id * id)
{
    if (!ma_context_is_loopback_supported(maCtx)) {
        L_FATAL("Audio loopback devices are not supported.");
        throw AudioException("Failed to initialise loopback device: backend not supported");
    }

    auto deviceConfig = ma_device_config_init(ma_device_type_loopback);
    deviceConfig.playback.pDeviceID = const_cast<ma_device_id *>(id);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.noClip = true;
    deviceConfig.dataCallback = readCallback;
    deviceConfig.pUserData = &audioContext; 

    L_INFO("Opening output device as loopback...");

    if (ma_device_init(maCtx, &deviceConfig, &device) != MA_SUCCESS) {
        L_FATAL("Failed to open output device...");
        throw AudioException("Failed to initialise miniaudio device");
    }

    deviceInit = true;
}

void AudioCapture::startStream() {

    if (ma_device_start(&device) != MA_SUCCESS) {
        L_FATAL("Failed to start audio device...");
        throw AudioException("Failed to start miniaudio device");
    }

}

void AudioCapture::closeStream()
{
    if (deviceInit) {
        L_INFO("Closing audio device...");
        ma_device_uninit(&device);
    }
    deviceInit = false;
}

void AudioCapture::setCaptureDuration(int nsamples) {
    audioContext.buffer.setCapacity(nsamples);
}

int AudioCapture::getSampleRate() const noexcept {
    return sampleRate;
}

void AudioCapture::readBlock(Eigen::ArrayXd & capture) noexcept {
    audioContext.buffer.readFrom(capture);
}

