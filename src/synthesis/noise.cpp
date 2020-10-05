#include "synthesis.h"
#include <ctime>
#include <random>

static std::mt19937 initGen()
{
    std::random_device rd;
    return std::mt19937(rd());
}

static auto gen = initGen();
static std::uniform_real_distribution<> dis(-1.0f, 1.0f);

std::vector<float> Synthesis::whiteNoise(int length)
{
    std::vector<float> out(length);
    for (int i = 0; i < length; ++i) {
        out[i] = dis(gen);
    }
    return out;
}
