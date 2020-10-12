#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include "../nodes/node/nodeio_frequencies.h"
#include "../analysis/filter/filter.h"
#include "../analysis/formant/formant.h"
#include <vector>
#include <deque>
#include <complex>

namespace Synthesis
{
    std::vector<float> whiteNoise(int length);
    std::vector<float> brownNoise(int length);
    std::vector<float> pinkNoise(int length);
    std::vector<float> aspirateNoise(int length);

    struct LF_State {
        double T0;
        double Rd;
        double te, tp, ta;
        double alpha, eps;
    };

    void lfRd2tetpta(LF_State& state);
    void lfEpsAlpha(LF_State& state);

    std::vector<float> lfGenFrame(float f0, float Fs, float Rd);

    std::vector<float> createPolynomialFromRoots(const std::vector<std::complex<float>>& z);

    std::vector<std::array<float, 6>> frequencyShiftFilter(const std::vector<Analysis::FormantData>& formants, float Fs, float factor);

    std::vector<float> filter(
                    const std::vector<float>& b,
                    const std::vector<float>& a,
                    const std::vector<float>& x,
                    std::vector<double>& zf);

    std::vector<float> sosfilter(
                    const std::vector<std::array<float, 6>>& sos,
                    const std::vector<float>& x,
                    std::vector<std::vector<double>>& zf);
}

#endif // SYNTHESIS_H
