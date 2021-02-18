#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include "rpcxx.h"
#include "../nodes/node/nodeio_frequencies.h"
#include "../analysis/filter/filter.h"
#include "../analysis/formant/formant.h"
#include <complex>

namespace Synthesis
{
    rpm::vector<double> whiteNoise(int length);
    rpm::vector<double> aspirateNoise(int length);

    struct LF_State {
        double T0;
        double Tc;
        double Rd;
        double te, tp, ta;
        double alpha, eps;
    };

    void lfRd2tetpta(LF_State& state);
    void lfEpsAlpha(LF_State& state);

    rpm::vector<double> lfGenFrame(double f0, double Fs, double Rd, double tc);

    rpm::vector<double> createPolynomialFromRoots(const rpm::vector<std::complex<double>>& z);

    rpm::vector<std::array<double, 6>> frequencyShiftFilter(const rpm::vector<Analysis::FormantData>& formants, double Fs, double factor);

    rpm::vector<double> filter(
                    const rpm::vector<double>& b,
                    const rpm::vector<double>& a,
                    const rpm::vector<double>& x,
                    rpm::vector<double>& zf);

    rpm::vector<double> sosfilter(
                    const rpm::vector<std::array<double, 6>>& sos,
                    const rpm::vector<double>& x,
                    rpm::vector<rpm::vector<double>>& zf);
}

#endif // SYNTHESIS_H
