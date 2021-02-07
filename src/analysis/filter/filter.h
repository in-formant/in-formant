#ifndef ANALYSIS_FILTER_H
#define ANALYSIS_FILTER_H

#include "rpcxx.h"
#include <array>
#include <complex>

namespace Analysis {

    rpm::vector<double> filter(const rpm::vector<double>& a, const rpm::vector<double>& x);
    
    rpm::vector<double> filter(const rpm::vector<double>& b, const rpm::vector<double>& a, const rpm::vector<double>& x);

    rpm::vector<std::array<double, 6>> butterworthHighpass(int N, double fc, double fs);
    rpm::vector<std::array<double, 6>> butterworthLowpass(int N, double fc, double fs);

    rpm::vector<std::array<double, 6>> zpk2sos(const rpm::vector<std::complex<double>>& z, const rpm::vector<std::complex<double>>& p, double k);

    rpm::vector<double> sosfilter(const rpm::vector<std::array<double, 6>>& sos, const rpm::vector<double>& x);

}

#endif // ANALYSIS_FILTER_H
