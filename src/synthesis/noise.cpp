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
static std::uniform_real_distribution<> dis(-1.0, 1.0);

rpm::vector<double> Synthesis::whiteNoise(int length)
{
    rpm::vector<double> out(length);
    for (int i = 0; i < length; ++i) {
        out[i] = dis(gen);
    }
    return out;
}

rpm::vector<double> Synthesis::aspirateNoise(int length)
{
    constexpr double alpha = -1.5;

    static rpm::vector<double> filter;
    static rpm::vector<double> zf;

    if (filter.size() == 0) {
        filter.resize(64);
        filter[0] = 1.0;
        for (int k = 1; k <= 63; ++k) {
            filter[k] = (k - 1.0 - alpha / 2.0) * filter[k - 1] / (double) k;
        }
    }

    return Synthesis::filter(filter, {1.0}, whiteNoise(length), zf);
}

