#ifndef ANALYSIS_UTIL_H
#define ANALYSIS_UTIL_H

#include <vector>
#include <complex>

#include "../formant/formant.h"

namespace Analysis {

    std::vector<int> findPeaks(const double *data, int length, int sign = +1);

    std::pair<std::vector<double>, std::vector<double>> findZerocros(const std::vector<double>& y, char m);

    std::pair<double, double> parabolicInterpolation(const std::vector<double>& array, int x);
    
    std::vector<std::complex<double>> findRoots(const std::vector<double>& p);

    FormantData calculateFormant(double r, double phi, double sampleRate);

    void sortFormants(std::vector<FormantData>& formants);

    void polishComplexRoot(const std::vector<double>& p, std::complex<double> *root, int maxIt);
    void polishRealRoot(const std::vector<double>& p, double *root, int maxIt);
    void polishRoots(const std::vector<double>& p, std::vector<std::complex<double>>& roots);

    void evaluatePolynomialWithDerivative(const std::vector<double>& p, double x, double *y, double *dy);
    void evaluatePolynomialWithDerivative(const std::vector<double>& p, const std::complex<double>& x, std::complex<double> *y, std::complex<double> *dy);
    
    std::vector<std::complex<double>> evaluatePolynomialDerivatives(const std::vector<std::complex<double>>& p, const std::complex<double>& x, int numberOfDerivatives);

}

#endif // ANALYSIS_UTIL_H
