#ifndef ANALYSIS_LAGUERRE_H
#define ANALYSIS_LAGUERRE_H

#include <vector>
#include <complex>

namespace Analysis {

    std::complex<double> laguerreRoot(const std::vector<std::complex<double>>& P, std::complex<double> xk, double accuracy);

    std::vector<std::complex<double>> laguerreDeflate(const std::vector<std::complex<double>>& P, const std::complex<double>& root);

    std::vector<std::complex<double>> laguerreSolve(const std::vector<double>& P);

}

#endif // ANALYSIS_LAGUERRE_H
