#ifndef ANALYSIS_UTIL_ABERTH_H
#define ANALYSIS_UTIL_ABERTH_H

#include <rpcxx.h>
#include <complex>

namespace Analysis {

rpm::vector<std::complex<double>> aberthRoots(const rpm::vector<double>& P);

}

#endif // ANALYSIS_UTIL_ABERTH_H


