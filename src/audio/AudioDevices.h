//
// Created by clo on 9/12/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIODEVICES_H
#define SPEECH_ANALYSIS_AUDIODEVICES_H

#include "miniaudio.h"
#include <string>
#include <memory>
#include "rpmalloc.h"

struct AudioDevice {
    ma_device_id id;
    std::string name;
};

class AudioDevices {
public:
    AudioDevices(ma_context * ctx);

    bool refreshList();

    const rpm::vector<AudioDevice> & getInputs() const;
    const rpm::vector<AudioDevice> & getOutputs() const;

private:
    ma_context * maCtx;

    rpm::vector<AudioDevice> inputs;
    rpm::vector<AudioDevice> outputs;
};

#endif // SPEECH_ANALYSIS_AUDIODEVICES_H
