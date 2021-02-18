#ifndef WITHOUT_SYNTH

#include "synthwrapper.h"
#include <QSplineSeries>
#include <cfloat>

using namespace Main;

SynthWrapper::SynthWrapper()
    : mEnabled(false),
      mNoiseGain(0.3),
      mGlotGain(1.0),
      mGlotPitch(170),
      mGlotRd(1.7),
      mGlotTc(1.0),
      mFilterShift(1.0),
      mVoiced(true),
      mFollowPitch(true),
      mFollowFormants(true),
      mFilterResponseDbMin(0),
      mFilterResponseDbMax(0),
      mSourceSpectrumDbMin(DBL_MAX),
      mSourceSpectrumDbMax(-DBL_MAX)
{
}

bool SynthWrapper::enabled() const
{
    return mEnabled;
}

double SynthWrapper::noiseGain() const
{
    return mNoiseGain;
}

double SynthWrapper::glotGain() const
{
    return mGlotGain;
}

double SynthWrapper::glotPitch() const
{
    return mGlotPitch;
}

double SynthWrapper::glotRd() const
{
    return mGlotRd;
}

double SynthWrapper::glotTc() const
{
    return mGlotTc;
}

double SynthWrapper::filterShift() const
{
    return mFilterShift;
}

bool SynthWrapper::voiced() const
{
    return mVoiced;
}

bool SynthWrapper::followPitch() const
{
    return mFollowPitch;
}

bool SynthWrapper::followFormants() const
{
    return mFollowFormants;
}

void SynthWrapper::setEnabled(bool b) 
{
    mEnabled = b;
    emit enabledChanged(b);
}

void SynthWrapper::setNoiseGain(double g)
{
    mNoiseGain = g;
    emit noiseGainChanged(g);
}

void SynthWrapper::setGlotGain(double g)
{
    mGlotGain = g;
    emit glotGainChanged(g);
}

void SynthWrapper::setGlotPitch(double pitch)
{
    mGlotPitch = pitch;
    emit glotPitchChanged(pitch);
}

void SynthWrapper::setGlotRd(double Rd)
{
    mGlotRd = Rd;
    emit glotRdChanged(Rd);
}

void SynthWrapper::setGlotTc(double Tc)
{
    mGlotTc = Tc;
    emit glotTcChanged(Tc);
}

void SynthWrapper::setFilterShift(double shf)
{
    mFilterShift = shf;
    emit filterShiftChanged(shf);
}

void SynthWrapper::setVoiced(bool v)
{
    mVoiced = v;
    emit voicedChanged(v);
}

void SynthWrapper::setFollowPitch(bool b)
{
    mFollowPitch = b;
    emit followPitchChanged(b);
}

void SynthWrapper::setFollowFormants(bool b)
{
    mFollowFormants = b;
    emit followFormantsChanged(b);
}

void SynthWrapper::setFilterResponse(const rpm::vector<double>& frequencies, const rpm::vector<double>& magnitudes)
{
    std::lock_guard<std::mutex> lock(mFilterResponseMutex);

    mFilterResponse.resize(frequencies.size());
    double dBmin = DBL_MAX;
    double dBmax = -DBL_MAX;
    for (int k = 0; k < frequencies.size(); ++k) {
        const double dB = 20.0 * log10(magnitudes[k]);
        mFilterResponse[k] = QPointF(frequencies[k], dB);
        if (dB < dBmin)
            dBmin = dB;
        if (dB > dBmax)
            dBmax = dB;
    }
    mFilterResponseDbMin = 10;

    if (mFilterResponseDbMax == 0) {
        mFilterResponseDbMax = dBmax;
    }
    else {
        if (dBmax > mFilterResponseDbMax) {
            mFilterResponseDbMax = 0.2 * mFilterResponseDbMax + 0.8 * dBmax;
        }
        else {
            mFilterResponseDbMax = 0.95 * mFilterResponseDbMax + 0.05 * dBmax;
        }
    }

    emit filterResponseChanged();
}

void SynthWrapper::setSource(const rpm::vector<double>& signal, double fs)
{
    std::lock_guard<std::mutex> lock(mSourceMutex);
    
    mSource.resize(signal.size());
    for (int k = 0; k < signal.size(); ++k) {
        mSource[k] = QPointF((k * 1000) / fs, signal[k]);
    }
    emit sourceChanged();
}

void SynthWrapper::setSourceSpectrum(const rpm::vector<double>& frequencies, const rpm::vector<double>& magnitudes)
{
    std::lock_guard<std::mutex> lock(mSourceSpectrumMutex);

    mSourceSpectrum.resize(frequencies.size());
    double dBmin = DBL_MAX;
    double dBmax = -DBL_MAX;
    for (int k = 0; k < frequencies.size(); ++k) {
        const double dB = 20.0 * log10(magnitudes[k]);
        mSourceSpectrum[k] = QPointF(frequencies[k], dB);
        if (dB < dBmin)
            dBmin = dB;
        if (dB > dBmax)
            dBmax = dB;
    }
    mSourceSpectrumDbMin = -60;
    mSourceSpectrumDbMax = std::max(mSourceSpectrumDbMax, dBmax);
    emit sourceSpectrumChanged();
}

void SynthWrapper::updateFilterResponseSeries(QXYSeries* series, QLogValueAxis *xAxis, QValueAxis* yAxis)
{
    std::lock_guard<std::mutex> lock(mFilterResponseMutex);
    
    series->replace(mFilterResponse);
    xAxis->setRange(100, mFilterResponse.back().x());

    double roundedMin = std::floor(mFilterResponseDbMin / 10) * 10;
    yAxis->setRange(roundedMin, mFilterResponseDbMax);
}

void SynthWrapper::updateSourceSeries(QXYSeries* series, QValueAxis* xAxis, QValueAxis* yAxis)
{
    std::lock_guard<std::mutex> lock(mSourceMutex);
    
    series->replace(mSource);
    xAxis->setRange(mSource.front().x(), mSource.back().x());
    yAxis->setRange(-1.5, 1.5);
}

void SynthWrapper::updateSourceSpectrumSeries(QXYSeries* series, QLogValueAxis *xAxis, QValueAxis* yAxis)
{
    std::lock_guard<std::mutex> lock(mSourceSpectrumMutex);
    
    series->replace(mSourceSpectrum);
    xAxis->setRange(100, mSourceSpectrum.back().x());
    
    double realMin = std::floor(mSourceSpectrumDbMin / 10) * 10;
    double realMax = std::ceil(mSourceSpectrumDbMax / 10) * 10;
    yAxis->setRange(realMin, realMax);
}

#endif // !WITHOUT_SYNTH
