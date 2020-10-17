#include "synthesis.h"
#include <ctime>
#include <random>

#ifdef __WIN32
#   define rd rand
#else
    static std::random_device rd;
#endif

#if SIZEOF_VOID_P == 4
    static std::mt19937 gen(rd());
#else
    static std::mt19937_64 gen(rd());
#endif
static std::uniform_real_distribution<> dis(-1.0f, 1.0f);

std::vector<double> Synthesis::whiteNoise(int length)
{
    std::vector<double> out(length);
    for (int i = 0; i < length; ++i) {
        out[i] = dis(gen);
    }
    return out;
}

std::vector<double> Synthesis::brownNoise(int length)
{
    static std::vector<double> filter;
    static std::vector<double> zf;

    if (filter.size() == 0) {
        filter.resize(64);
        filter[0] = 1.0f;
        for (int k = 1; k <= 63; ++k) {
            filter[k] = (k - 2.0f) * filter[k - 1] / (double) k;
        }
    }

    return Synthesis::filter(filter, {1.0f}, whiteNoise(length), zf);
}

std::vector<double> Synthesis::pinkNoise(int length)
{
    static std::vector<double> filter;
    static std::vector<double> zf;

    if (filter.size() == 0) {
        filter.resize(64);
        filter[0] = 1.0f;
        for (int k = 1; k <= 63; ++k) {
            filter[k] = (k - 1.5f) * filter[k - 1] / (double) k;
        }
    }

    return Synthesis::filter(filter, {1.0f}, whiteNoise(length), zf);
}

std::vector<double> Synthesis::aspirateNoise(int length)
{
    constexpr double alpha = 1.5;

    static std::vector<double> filter;
    static std::vector<double> zf;

    if (filter.size() == 0) {
        filter.resize(64);
        filter[0] = 1.0f;
        for (int k = 1; k <= 63; ++k) {
            filter[k] = (k - 1.0f - alpha / 2.0f) * filter[k - 1] / (double) k;
        }
    }

    return Synthesis::filter(filter, {1.0f}, whiteNoise(length), zf);
}

