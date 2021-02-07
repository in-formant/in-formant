#ifndef ANALYSIS_LAGUERRE_H
#define ANALYSIS_LAGUERRE_H

#include "rpcxx.h"
#include <complex>

namespace Analysis {

    std::complex<double> laguerreRoot(const rpm::vector<std::complex<double>>& P, std::complex<double> xk, double accuracy);

    rpm::vector<std::complex<double>> laguerreDeflate(const rpm::vector<std::complex<double>>& P, const std::complex<double>& root);

    rpm::vector<std::complex<double>> laguerreSolve(const rpm::vector<double>& P);

}

#endif // ANALYSIS_LAGUERRE_H
