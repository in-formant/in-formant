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
        void setGlotPitch(float);
        void setGlotRd(float);
        void setFilter(const std::vector<float>&, float);

        float getMasterGain() const;
        float getNoiseGain() const;
        float getGlotGain() const;
        float getGlotPitch() const;
        float getGlotRd() const;

        void generateAudio(int requestedLength);

        static void audioCallback(float *, int, void *);

    private:
        Module::Audio::Queue *playbackQueue;

        float masterGain;
        float noiseGain;
        float glotGain;
        float glotPitch;
        float glotRd;
        std::vector<float> filter;
        
        float realMasterGain;
        float realNoiseGain;
        float realGlotGain;
        float realGlotPitch;
        float realGlotRd;
        std::vector<float> realFilter;

        std::deque<float> filterMemoryNoise;
        std::deque<float> filterMemoryGlot;

        std::vector<float> glotSurplus;
        std::vector<float> surplus;

        Module::Audio::Resampler noiseResampler;
        Module::Audio::Resampler outputResampler;
    };
}

#endif // APP_SYNTH_H
