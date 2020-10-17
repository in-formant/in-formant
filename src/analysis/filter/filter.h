#ifndef ANALYSIS_FILTER_H
#define ANALYSIS_FILTER_H

#include <array>
#include <vector>
#include <complex>

namespace Analysis {

    std::vector<double> filter(const std::vector<double>& a, const std::vector<double>& x);
    
    std::vector<double> filter(const std::vector<double>& b, const std::vector<double>& a, const std::vector<double>& x);

    std::vector<std::array<double, 6>> butterworthHighpass(int N, double fc, double fs);
    std::vector<std::array<double, 6>> butterworthLowpass(int N, double fc, double fs);

    std::vector<std::array<double, 6>> zpk2sos(const std::vector<std::complex<double>>& z, const std::vector<std::complex<double>>& p, double k);

    std::vector<double> sosfilter(const std::vector<std::array<double, 6>>& sos, const std::vector<double>& x);

}

#endif // ANALYSIS_FILTER_H
