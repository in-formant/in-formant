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
    std::vector<double> whiteNoise(int length);
    std::vector<double> brownNoise(int length);
    std::vector<double> pinkNoise(int length);
    std::vector<double> aspirateNoise(int length);

    struct LF_State {
        double T0;
        double Tc;
        double Rd;
        double te, tp, ta;
        double alpha, eps;
    };

    void lfRd2tetpta(LF_State& state);
    void lfEpsAlpha(LF_State& state);

    std::vector<double> lfGenFrame(double f0, double Fs, double Rd, double tc);

    std::vector<double> createPolynomialFromRoots(const std::vector<std::complex<double>>& z);

    std::vector<std::array<double, 6>> frequencyShiftFilter(const std::vector<Analysis::FormantData>& formants, double Fs, double factor);

    std::vector<double> filter(
                    const std::vector<double>& b,
                    const std::vector<double>& a,
                    const std::vector<double>& x,
                    std::vector<double>& zf);

    std::vector<double> sosfilter(
                    const std::vector<std::array<double, 6>>& sos,
                    const std::vector<double>& x,
                    std::vector<std::vector<double>>& zf);
}

#endif // SYNTHESIS_H
