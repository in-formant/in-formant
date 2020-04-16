#ifndef SINE_WAVE_H
#define SINE_WAVE_H

#include "miniaudio.h"
#include "rpmalloc.h"

#define MAX_SINE_BUFFER 1024

class SineWave {
public:
    SineWave();
    ~SineWave();

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

    double mSampleRateIn;
    double mSampleRateOut;
    
    double mGain;
    double mFrequency;
    
    double mTimeInSamples;
    double mPeriodInSamples;

    rpm::vector<float> sineIn, sineOut;

    ma_resampler resampler;
};

#endif // SINE_WAVE_H
