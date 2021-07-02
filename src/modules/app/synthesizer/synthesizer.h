#ifndef APP_SYNTH_H
#define APP_SYNTH_H

#ifndef WITHOUT_SYNTH

#include "rpcxx.h"

#include "../../../analysis/analysis.h"
#include "../../../synthesis/synthesis.h"
#include "../../audio/audio.h"

#include <mutex>
#include <QObject>

namespace Module::App
{
    class Synthesizer : public QObject {
        Q_OBJECT

    public:
        Synthesizer(Module::Audio::Queue *);

    public slots:
        void setMasterGain(double);
        void setNoiseGain(double);
        void setGlotGain(double);
        void setGlotPitch(double);
        void setGlotRd(double);
        void setGlotTc(double);
        void setFormants(const rpm::vector<Analysis::FormantData>&);
        void setFilterShift(double);
        void setVoiced(bool);

    public:
        double getMasterGain() const;
        double getNoiseGain() const;
        double getGlotGain() const;
        double getGlotPitch() const;
        double getGlotRd() const;
        double getGlotTc() const;
        double getFilterShift() const;
        bool isVoiced() const;
        
        rpm::vector<std::array<double, 6>> getFilterCopy(double *fs) const;
        rpm::vector<double> getSourceCopy(double fs, double durationInMs) const;
        
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
        mutable std::mutex realFilterMutex;

        rpm::vector<rpm::vector<double>> zfNoise;
        rpm::vector<rpm::vector<double>> zfGlot;

        rpm::vector<double> glotSurplus;
        rpm::vector<double> surplus;

        Module::Audio::Resampler resampler;
    };
}

#endif // !WITHOUT_SYNTH

#endif // APP_SYNTH_H
