//
// Created by clo on 9/12/2019.
//

#include <iostream>
#include "AudioDevices.h"

AudioDevices::AudioDevices()
{
    refreshList();
}

bool AudioDevices::refreshList()
{
    inputs.clear();
    outputs.clear();

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "ERROR: Pa_CountDevices returned 0x" << std::hex << numDevices << std::dec << std::endl;
        return false;
    }

    const PaDeviceInfo * info;

    for (int id = 0; id < numDevices; ++id) {
        info = Pa_GetDeviceInfo(id);

        if (info->maxInputChannels > 0) {
            inputs.push_back({
                .id = id,
                .name = info->name,
            });
        }

        if (info->maxOutputChannels > 0) {
            outputs.push_back({
                .id = id,
                .name = info->name
            });
        }
    }

    return true;
}

const std::vector<AudioDevice> & AudioDevices::getInputs() const {
    return inputs;
}

const std::vector<AudioDevice> & AudioDevices::getOutputs() const {
    return outputs;
}
