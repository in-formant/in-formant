//
// Created by clo on 9/12/2019.
//

#include <iostream>
#include "../Exceptions.h"
#include "AudioDevices.h"

AudioDevices::AudioDevices()
{
    std::vector<ma_backend> backends{
        ma_backend_dsound,
        ma_backend_winmm,
        ma_backend_wasapi,

        ma_backend_coreaudio,
        ma_backend_sndio,
        ma_backend_audio4,

        ma_backend_alsa,
        ma_backend_jack,
        ma_backend_pulseaudio,
        ma_backend_oss,

        ma_backend_aaudio,
        ma_backend_opensl,
        ma_backend_webaudio,
        ma_backend_null
    };

    ma_context_config ctxCfg = ma_context_config_init();
    ctxCfg.threadPriority = ma_thread_priority_realtime;
    ctxCfg.alsa.useVerboseDeviceEnumeration = true;
    ctxCfg.pulse.tryAutoSpawn = true;
    ctxCfg.jack.tryStartServer = true;

    if (ma_context_init(backends.data(), backends.size(), &ctxCfg, &maCtx) != MA_SUCCESS) {
        throw AudioException("Failed to initialise miniaudio context");
    }

    refreshList();
}

ma_context * AudioDevices::getContext()
{
    return &maCtx;
}

bool AudioDevices::refreshList()
{
    inputs.clear();
    outputs.clear();

    ma_device_info *playback, *capture;
    ma_uint32 playbackCount, captureCount;

    ma_context_get_devices(&maCtx, &playback, &playbackCount, &capture, &captureCount);

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
        std::cout << "No input devices found." << std::endl;
    }

    if (outputs.empty()) {
        std::cout << "No output devices found." << std::endl;
    }

    return true;
}

const std::vector<AudioDevice> & AudioDevices::getInputs() const {
    return inputs;
}

const std::vector<AudioDevice> & AudioDevices::getOutputs() const {
    return outputs;
}

