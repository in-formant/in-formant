#ifndef NOISE_FILTER_H
#define NOISE_FILTER_H

#include <Eigen/Core>
#include "miniaudio.h"
#include <mutex>
#include "rpmalloc.h"

#define MAX_NOISE_BUFFER 1024

class NoiseFilter {
public:
    NoiseFilter();
    ~NoiseFilter();
    
    void initOutput(int sampleRate, int numChannels);

    void setPlaying(bool playing);
    void setFilter(int sampleRateIn, const Eigen::ArrayXd & spec);

    void readFrames(float *output, int frameCount);

private:
    void applyFilter(rpm::vector<float>& noiseIn, rpm::vector<float>& noiseOut);

    int mChannels;
    int mSampleRateIn;
    int mSampleRateOut;

    double gainDecay;
    double filterDecay;

    double targetGain;
    Eigen::ArrayXd targetFilter;
    double mGain;

    int filterPassCount;
    int nfft;
    Eigen::ArrayXd filter;
    std::mutex mutex;

    ma_noise noise;
    ma_resampler resampler;

    rpm::vector<float> noiseIn, noiseOut, noiseRes;

    rpm::vector<rpm::deque<float>> memoryOut;
};

#endif // NOISE_FILTER_H
