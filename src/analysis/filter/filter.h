#ifndef ANALYSIS_FILTER_H
#define ANALYSIS_FILTER_H

#include <array>
#include <vector>
#include <complex>

namespace Analysis {

    std::vector<float> filter(const std::vector<float>& a, const std::vector<float>& x);
    
    std::vector<float> filter(const std::vector<float>& b, const std::vector<float>& a, const std::vector<float>& x);

    std::vector<std::array<float, 6>> butterworth(int N, float fc, float fs);

    std::vector<std::array<float, 6>> zpk2sos(const std::vector<std::complex<float>>& z, const std::vector<std::complex<float>>& p, float k);

    std::vector<float> sosfilter(const std::vector<std::array<float, 6>>& sos, const std::vector<float>& x);

}

#endif // ANALYSIS_FILTER_H
