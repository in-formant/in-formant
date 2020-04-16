#ifndef NOISE_FILTER_H
#define NOISE_FILTER_H

#include <Eigen/Core>
#include "miniaudio.h"
#include <vector>
#include <deque>
#include <mutex>

#define MAX_BUFFER_SIZE 2048

class NoiseFilter {
public:
    NoiseFilter();
    ~NoiseFilter();
    
    void initOutput(int sampleRate, int numChannels);

    void setPlaying(bool playing);
    void setFilter(int sampleRateIn, const Eigen::ArrayXd & spec);

    void readFrames(float *output, int frameCount);

private:
    void applyFilter();

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

    std::vector<float> noiseIn;
    std::vector<float> noiseOut;
    std::vector<float> noiseRes;

    std::vector<std::deque<float>> memoryOut;
};

#endif // NOISE_FILTER_H
