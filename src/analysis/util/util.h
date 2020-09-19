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

    void polishComplexRoot(const std::vector<float>& p, std::complex<float> *root, int maxIt);
    void polishRealRoot(const std::vector<float>& p, float *root, int maxIt);
    void polishRoots(const std::vector<float>& p, std::vector<std::complex<float>>& roots);

    void evaluatePolynomialWithDerivative(const std::vector<float>& p, float x, float *y, float *dy);
    void evaluatePolynomialWithDerivative(const std::vector<float>& p, const std::complex<float>& x, std::complex<float> *y, std::complex<float> *dy);
    
    std::vector<std::complex<double>> evaluatePolynomialDerivatives(const std::vector<std::complex<double>>& p, const std::complex<double>& x, int numberOfDerivatives);

}

#endif // ANALYSIS_UTIL_H
