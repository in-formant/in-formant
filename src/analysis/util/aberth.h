#ifndef ANALYSIS_UTIL_ABERTH_H
#define ANALYSIS_UTIL_ABERTH_H

#include <rpcxx.h>
#include <complex>

namespace Analysis {

rpm::vector<std::complex<double>> aberthRoots(const rpm::vector<double>& P);

rpm::vector<std::complex<double>> aberthRootsAroundInitial(
        const rpm::vector<double>& P, double r, double phi, int count);

}

#endif // ANALYSIS_UTIL_ABERTH_H


