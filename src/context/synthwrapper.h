#ifndef SYNTH_WRAPPER_H
#define SYNTH_WRAPPER_H

#ifndef WITHOUT_SYNTH

#include <QObject>
#include "../modules/app/synthesizer/synthesizer.h"

#include <QXYSeries>
#include <QValueAxis>
#include <QLogValueAxis>

using namespace QtCharts;

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
        Q_PROPERTY(bool followPitch     READ followPitch    WRITE setFollowPitch    NOTIFY followPitchChanged)
        Q_PROPERTY(bool followFormants  READ followFormants WRITE setFollowFormants NOTIFY followFormantsChanged)
        Q_PROPERTY(QVector<QPointF> filterResponse MEMBER mFilterResponse NOTIFY filterResponseChanged)
        Q_PROPERTY(QVector<QPointF> source         MEMBER mSource         NOTIFY sourceChanged)
        Q_PROPERTY(QVector<QPointF> sourceSpectrum MEMBER mSourceSpectrum NOTIFY sourceSpectrumChanged)

    signals:
        void enabledChanged(bool);
        void noiseGainChanged(double);
        void glotGainChanged(double);
        void glotPitchChanged(double);
        void glotRdChanged(double);
        void glotTcChanged(double);
        void filterShiftChanged(double);
        void voicedChanged(double);
        void followPitchChanged(bool);
        void followFormantsChanged(bool);
        void filterResponseChanged();
        void sourceChanged();
        void sourceSpectrumChanged();

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
        bool followPitch() const;
        bool followFormants() const;

        void setEnabled(bool);
        void setNoiseGain(double);
        void setGlotGain(double);
        void setGlotPitch(double);
        void setGlotRd(double);
        void setGlotTc(double);
        void setFilterShift(double);
        void setVoiced(bool);
        void setFollowPitch(bool);
        void setFollowFormants(bool);
        
        void setFilterResponse(const rpm::vector<double>& frequencies, const rpm::vector<double>& magnitudes);
        void setSource(const rpm::vector<double>& signal, double fs);
        void setSourceSpectrum(const rpm::vector<double>& frequencies, const rpm::vector<double>& magnitudes);

        Q_INVOKABLE void updateFilterResponseSeries(QXYSeries* series, QLogValueAxis *xAxis, QValueAxis* yAxis);
        Q_INVOKABLE void updateSourceSeries(QXYSeries* series, QValueAxis *xAxis, QValueAxis* yAxis);
        Q_INVOKABLE void updateSourceSpectrumSeries(QXYSeries* series, QLogValueAxis *xAxis, QValueAxis* yAxis);

    private:
        void evaluateFilterResponse();

        bool mEnabled;
        double mNoiseGain;
        double mGlotGain;
        double mGlotPitch;
        double mGlotRd;
        double mGlotTc;
        double mFilterShift;
        bool mVoiced;
        bool mFollowPitch;
        bool mFollowFormants;

        QVector<QPointF> mFilterResponse;
        double mFilterResponseDbMin;
        double mFilterResponseDbMax;
        QMutex mFilterResponseMutex;

        QVector<QPointF> mSource;
        QMutex mSourceMutex;

        QVector<QPointF> mSourceSpectrum;
        double mSourceSpectrumDbMin;
        double mSourceSpectrumDbMax;
        QMutex mSourceSpectrumMutex;
    };

}

#endif // !WITHOUT_SYNTH

#endif // SYNTH_WRAPPER_H
