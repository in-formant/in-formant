#ifndef SINE_WAVE_H
#define SINE_WAVE_H

#include "miniaudio.h"

class SineWave {
public:
    SineWave();
    
    void initWaveform(int sampleRate, int numChannels);
    
    void setPlaying(bool playing);
    void setFrequency(double frequency);

    void readFrames(float *output, int frameCount);

private:
    int mChannels;
    double gainDecay;
    double frequencyDecay;

    double targetGain;
    double targetFrequency;

    double mSampleRate;

    double mTimeInSamples;
    
    double mGain;
    
    double mFrequency;
    double mPeriodInSamples;

    ma_waveform waveform;
};

#endif // SINE_WAVE_H
