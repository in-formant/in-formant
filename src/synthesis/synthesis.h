#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include "../nodes/node/nodeio_frequencies.h"
#include "../analysis/filter/filter.h"
#include "../analysis/formant/formant.h"
#include <vector>
#include <deque>

namespace Synthesis
{
    std::vector<float> whiteNoise(int length);

    struct LF_State {
        double T0;
        double Rd;
        double te, tp, ta;
        double alpha, eps;
    };

    void lfRd2tetpta(LF_State& state);
    void lfEpsAlpha(LF_State& state);

    std::vector<float> lfGenFrame(float f0, float Fs, float Rd);

    std::vector<float> frequencyShiftFilter(const std::vector<float>& a, float Fs, float factor);

    std::vector<float> filter(
                    const std::vector<float>& b,
                    const std::vector<float>& a,
                    const std::vector<float>& x,
                    std::deque<float>& memoryOut);
}

#endif // SYNTHESIS_H
