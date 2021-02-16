#include "synthwrapper.h"

using namespace Main;

SynthWrapper::SynthWrapper()
    : mEnabled(false),
      mNoiseGain(0.3),
      mGlotGain(1.0),
      mGlotPitch(170),
      mGlotRd(1.7),
      mGlotTc(1.0),
      mFilterShift(1.0),
      mVoiced(true)
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
