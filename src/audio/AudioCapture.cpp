//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AudioCapture.h"
#include "../Exceptions.h"

AudioCapture::AudioCapture(ma_context * maCtx)
    : sampleRate(48000), maCtx(maCtx)
{
    audioContext.sampleRate = sampleRate;
    audioContext.buffer.setCapacity(BUFFER_SAMPLE_COUNT(sampleRate));
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
    deviceConfig.capture.shareMode = ma_share_mode_exclusive;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.performanceProfile = ma_performance_profile_low_latency;
    deviceConfig.noClip = true;
    deviceConfig.dataCallback = readCallback;
    deviceConfig.pUserData = &audioContext;
   
    if (ma_device_init(maCtx, &deviceConfig, &device) != MA_SUCCESS) {
        // Try again with shared mode
        deviceConfig.capture.shareMode = ma_share_mode_shared;

        if (ma_device_init(maCtx, &deviceConfig, &device) != MA_SUCCESS) {
            throw AudioException("Failed to initialise miniaudio device");
        }
    }
}

void AudioCapture::openOutputDevice(const ma_device_id * id)
{
    if (!ma_context_is_loopback_supported(maCtx)) {
        throw AudioException("Failed to initialise loopback device: backend not supported");
    }

    auto deviceConfig = ma_device_config_init(ma_device_type_loopback);
    deviceConfig.playback.pDeviceID = const_cast<ma_device_id *>(id);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.playback.shareMode = ma_share_mode_exclusive;
    deviceConfig.sampleRate = sampleRate;
    deviceConfig.performanceProfile = ma_performance_profile_low_latency;
    deviceConfig.noClip = true;
    deviceConfig.dataCallback = readCallback;
    deviceConfig.pUserData = &audioContext;
   
    audioContext.sampleRate = sampleRate;
    audioContext.buffer.setCapacity(BUFFER_SAMPLE_COUNT(sampleRate));

    if (ma_device_init(maCtx, &deviceConfig, &device) != MA_SUCCESS) {
        // Try again with shared mode
        deviceConfig.playback.shareMode = ma_share_mode_shared;

        if (ma_device_init(maCtx, &deviceConfig, &device) != MA_SUCCESS) {
            throw AudioException("Failed to initialise miniaudio device");
        }
    }
}

void AudioCapture::startStream() {

    if (ma_device_start(&device) != MA_SUCCESS) {
        throw AudioException("Failed to start miniaudio device");
    }

}

void AudioCapture::closeStream()
{
    ma_device_uninit(&device);
}


int AudioCapture::getSampleRate() const noexcept {
    return sampleRate;
}

void AudioCapture::readBlock(Eigen::ArrayXd & capture) noexcept {
    capture.conservativeResize(CAPTURE_SAMPLE_COUNT(sampleRate));
    audioContext.buffer.readFrom(capture);
}
