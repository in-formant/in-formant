#ifndef SINE_WAVE_H
#define SINE_WAVE_H

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
    
    double mGain;
    double mFrequency;
    
    double mTimeInSamples;
    double mPeriodInSamples;
};

#endif // SINE_WAVE_H
