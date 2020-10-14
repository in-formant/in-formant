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
        void setGlotTc(float);
        void setFormants(const std::vector<Analysis::FormantData>&);
        void setFilterShift(float);
        void setVoiced(bool);

        float getMasterGain() const;
        float getNoiseGain() const;
        float getGlotGain() const;
        float getGlotPitch() const;
        float getGlotRd() const;
        float getGlotTc() const;
        float getFilterShift() const;

        void generateAudio(int requestedLength);

        static void audioCallback(float *, int, void *);

    private:
        Module::Audio::Queue *playbackQueue;

        float masterGain;
        float noiseGain;
        float glotGain;
        float glotPitch;
        float glotRd;
        float glotTc;
        std::vector<Analysis::FormantData> formants;
        float filterShift;
        bool voiced;

        float realMasterGain;
        float realNoiseGain;
        float realGlotGain;
        float realGlotPitch;
        float realGlotRd;
        float realGlotTc;
        std::vector<Analysis::FormantData> realFormants;
        std::vector<std::array<float, 6>> realFilter;
        float realFilterShift;

        std::vector<std::vector<double>> zfNoise;
        std::vector<std::vector<double>> zfGlot;

        std::vector<float> glotSurplus;
        std::vector<float> surplus;

        Module::Audio::Resampler resampler;
    };
}

#endif // APP_SYNTH_H
