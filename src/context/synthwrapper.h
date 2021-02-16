#ifndef SYNTH_WRAPPER_H
#define SYNTH_WRAPPER_H

#include <QObject>
#include "../modules/app/synthesizer/synthesizer.h"

namespace Main {
   
    class SynthWrapper : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool enabled         READ enabled        WRITE setEnabled        NOTIFY enabledChanged)
        Q_PROPERTY(double noiseGain     READ noiseGain      WRITE setNoiseGain      NOTIFY noiseGainChanged)
        Q_PROPERTY(double glotGain      READ glotGain       WRITE setGlotGain       NOTIFY glotGainChanged)
        Q_PROPERTY(double glotPitch     READ glotPitch      WRITE setGlotPitch      NOTIFY glotPitchChanged)
        Q_PROPERTY(double glotRd        READ glotRd         WRITE setGlotRd         NOTIFY glotRdChanged)
        Q_PROPERTY(double glotTc        READ glotTc         WRITE setGlotTc         NOTIFY glotTcChanged)
        Q_PROPERTY(double filterShift   READ filterShift    WRITE setFilterShift    NOTIFY filterShiftChanged)
        Q_PROPERTY(bool voiced          READ voiced         WRITE setVoiced         NOTIFY voicedChanged)

    signals:
        void enabledChanged(bool);
        void noiseGainChanged(double);
        void glotGainChanged(double);
        void glotPitchChanged(double);
        void glotRdChanged(double);
        void glotTcChanged(double);
        void filterShiftChanged(double);
        void voicedChanged(double);

    public:
        SynthWrapper();

        bool enabled() const;
        double noiseGain() const;
        double glotGain() const;
        double glotPitch() const;
        double glotRd() const;
        double glotTc() const;
        double filterShift() const;
        bool voiced() const;

        void setEnabled(bool);
        void setNoiseGain(double);
        void setGlotGain(double);
        void setGlotPitch(double);
        void setGlotRd(double);
        void setGlotTc(double);
        void setFilterShift(double);
        void setVoiced(bool);

    private:
        bool mEnabled;
        double mNoiseGain;
        double mGlotGain;
        double mGlotPitch;
        double mGlotRd;
        double mGlotTc;
        double mFilterShift;
        bool mVoiced;
    };

}

#endif // SYNTH_WRAPPER_H
