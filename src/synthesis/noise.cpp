#include "synthesis.h"
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(-1.0f, 1.0f);

std::vector<float> Synthesis::whiteNoise(int length)
{
    std::vector<float> out(length);
    for (int i = 0; i < length; ++i) {
        out[i] = dis(gen);
    }
    return out;
}

std::vector<float> Synthesis::brownNoise(int length, float lastSample)
{
    std::vector<float> out = whiteNoise(length);
        
    for (int i = 0; i < length; ++i) {
        out[i] = out[i] + 0.96 * lastSample;
        lastSample = out[i];
    }

    return out;
}
