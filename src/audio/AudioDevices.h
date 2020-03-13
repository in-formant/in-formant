//
// Created by clo on 9/12/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIODEVICES_H
#define SPEECH_ANALYSIS_AUDIODEVICES_H

#include "miniaudio.h"
#include <string>
#include <memory>
#include <vector>

struct AudioDevice {
    ma_device_id id;
    std::string name;
};

class AudioDevices {
public:
    AudioDevices(ma_context * ctx);

    bool refreshList();

    const std::vector<AudioDevice> & getInputs() const;
    const std::vector<AudioDevice> & getOutputs() const;

private:
    ma_context * maCtx;

    std::vector<AudioDevice> inputs;
    std::vector<AudioDevice> outputs;
};

#endif // SPEECH_ANALYSIS_AUDIODEVICES_H
