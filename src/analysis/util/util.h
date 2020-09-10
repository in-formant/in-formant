#ifndef ANALYSIS_UTIL_H
#define ANALYSIS_UTIL_H

#include <vector>
#include <complex>

#include "../formant/formant.h"

namespace Analysis {

    std::vector<int> findPeaks(const float *data, int length, int sign = +1);

    std::vector<std::complex<float>> findRoots(const std::vector<float>& p);

    FormantData calculateFormant(float r, float phi, float sampleRate);

    void sortFormants(std::vector<FormantData>& formants);

}

#endif // ANALYSIS_UTIL_H
