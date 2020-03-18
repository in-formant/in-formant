//
// Created by clo on 9/12/2019.
//

#include <iostream>
#include "../Exceptions.h"
#include "AudioDevices.h"
#include "../log/simpleQtLogger.h"

AudioDevices::AudioDevices(ma_context * ctx)
    : maCtx(ctx)
{
    refreshList();
}

bool AudioDevices::refreshList()
{
    L_INFO("Refreshing audio device list...");

    inputs.clear();
    outputs.clear();

    ma_device_info *playback, *capture;
    ma_uint32 playbackCount, captureCount;

    ma_context_get_devices(maCtx, &playback, &playbackCount, &capture, &captureCount);

    for (int i = 0; i < playbackCount; ++i) {
        ma_device_info info = playback[i];
        
        outputs.push_back({
            .id = info.id,
            .name = info.name,
        });
    }

    for (int i = 0; i < captureCount; ++i) {
        ma_device_info info = capture[i];
        
        inputs.push_back({
            .id = info.id,
            .name = info.name,
        });
    }

    if (inputs.empty()) {
        L_WARN("No input devices found.");
    }

    if (outputs.empty()) {
        L_WARN("No output devices found.");
    }

    return true;
}

const std::vector<AudioDevice> & AudioDevices::getInputs() const {
    return inputs;
}

const std::vector<AudioDevice> & AudioDevices::getOutputs() const {
    return outputs;
}

