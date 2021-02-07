#ifndef ANALYSIS_UTIL_H
#define ANALYSIS_UTIL_H

#include "rpcxx.h"

#include <complex>

#include "../formant/formant.h"

namespace Analysis {

    rpm::vector<int> findPeaks(const double *data, int length, int sign = +1);

    std::pair<rpm::vector<double>, rpm::vector<double>> findZerocros(const rpm::vector<double>& y, char m);

    std::pair<double, double> parabolicInterpolation(const rpm::vector<double>& array, int x);
    
    rpm::vector<std::complex<double>> findRoots(const rpm::vector<double>& p);

    FormantData calculateFormant(double r, double phi, double sampleRate);

    void sortFormants(rpm::vector<FormantData>& formants);

    void polishComplexRoot(const rpm::vector<double>& p, std::complex<double> *root, int maxIt);
    void polishRealRoot(const rpm::vector<double>& p, double *root, int maxIt);
    void polishRoots(const rpm::vector<double>& p, rpm::vector<std::complex<double>>& roots);

    void evaluatePolynomialWithDerivative(const rpm::vector<double>& p, double x, double *y, double *dy);
    void evaluatePolynomialWithDerivative(const rpm::vector<double>& p, const std::complex<double>& x, std::complex<double> *y, std::complex<double> *dy);
    
    rpm::vector<std::complex<double>> evaluatePolynomialDerivatives(const rpm::vector<std::complex<double>>& p, const std::complex<double>& x, int numberOfDerivatives);

}

#endif // ANALYSIS_UTIL_H
