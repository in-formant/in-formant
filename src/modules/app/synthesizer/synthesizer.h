#ifndef APP_SYNTH_H
#define APP_SYNTH_H

#include "../../../analysis/analysis.h"
#include "../../../synthesis/synthesis.h"
#include "../../audio/audio.h"

namespace Module::App
{
    class Synthesizer {
    public:
        Synthesizer(Module::Audio::Queue *);

        void initialize();

        void setMasterGain(float);
        void setNoiseGain(float);
        void setGlotGain(float);
        void setFilter(const std::vector<float>&, float);

        void generateAudio(int requestedLength);

        static void audioCallback(float *, int, void *);

    private:
        Module::Audio::Queue *playbackQueue;

        float masterGain;
        float noiseGain;
        float glotGain;
        std::vector<float> filter;
        
        float realMasterGain;
        float realNoiseGain;
        float realGlotGain;
        std::vector<float> realFilter;

        std::deque<float> filterMemory;

        std::vector<float> surplus;

        Module::Audio::Resampler noiseResampler;
        Module::Audio::Resampler outputResampler;
    };
}

#endif // APP_SYNTH_H
