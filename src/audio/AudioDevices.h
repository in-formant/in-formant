//
// Created by clo on 9/12/2019.
//

#ifndef SPEECH_ANALYSIS_AUDIODEVICES_H
#define SPEECH_ANALYSIS_AUDIODEVICES_H

#include <portaudio.h>
#include <string>
#include <memory>
#include <vector>

struct AudioDevice {
    int id;
    std::string name;
};

class AudioDevices {
public:
    AudioDevices();

    bool refreshList();

    const std::vector<AudioDevice> & getInputs() const;
    const std::vector<AudioDevice> & getOutputs() const;

    PaDeviceIndex getDefaultInputDevice() const;

private:
    std::vector<AudioDevice> inputs;
    std::vector<AudioDevice> outputs;
};

#endif // SPEECH_ANALYSIS_AUDIODEVICES_H
