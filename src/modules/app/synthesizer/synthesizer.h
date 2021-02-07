#ifndef APP_SYNTH_H
#define APP_SYNTH_H

#include "rpcxx.h"

#include "../../../analysis/analysis.h"
#include "../../../synthesis/synthesis.h"
#include "../../audio/audio.h"

namespace Module::App
{
    class Synthesizer {
    public:
        Synthesizer(Module::Audio::Queue *);
        virtual ~Synthesizer();

        void initialize();

        void setMasterGain(double);
        void setNoiseGain(double);
        void setGlotGain(double);
        void setGlotPitch(double);
        void setGlotRd(double);
        void setGlotTc(double);
        void setFormants(const rpm::vector<Analysis::FormantData>&);
        void setFilterShift(double);
        void setVoiced(bool);

        double getMasterGain() const;
        double getNoiseGain() const;
        double getGlotGain() const;
        double getGlotPitch() const;
        double getGlotRd() const;
        double getGlotTc() const;
        double getFilterShift() const;
        bool isVoiced() const;
        
        void generateAudio(int requestedLength);

        static void audioCallback(double *, int, void *);

    private:
        Module::Audio::Queue *playbackQueue;

        double masterGain;
        double noiseGain;
        double glotGain;
        double glotPitch;
        double glotRd;
        double glotTc;
        rpm::vector<Analysis::FormantData> formants;
        double filterShift;
        bool voiced;

        double realMasterGain;
        double realNoiseGain;
        double realGlotGain;
        double realGlotPitch;
        double realGlotRd;
        double realGlotTc;
        rpm::vector<Analysis::FormantData> realFormants;
        rpm::vector<std::array<double, 6>> realFilter;
        double realFilterShift;

        rpm::vector<rpm::vector<double>> zfNoise;
        rpm::vector<rpm::vector<double>> zfGlot;

        rpm::vector<double> glotSurplus;
        rpm::vector<double> surplus;

        Module::Audio::Resampler *resampler;
    };
}

#endif // APP_SYNTH_H
