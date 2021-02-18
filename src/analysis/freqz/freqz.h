#ifndef ANALYSIS_FREQZ_H
#define ANALYSIS_FREQZ_H

#include "rpcxx.h"
#include <utility>
#include <complex>
#include <array>

namespace Analysis
{
    rpm::vector<double> sosfreqz(const rpm::vector<std::array<double, 6>>& sos, const rpm::vector<std::complex<double>>& wn);
}

#endif // ANALYSIS_FREQZ_H
