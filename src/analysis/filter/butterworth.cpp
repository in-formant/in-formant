#include "filter.h"
#include "../../synthesis/synthesis.h"
#include <complex>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

using namespace Eigen;

VectorXcf poly(const VectorXcf& z) {
    VectorXcf poly(z.size() + 1);
    poly.setOnes();

    poly(0) = 1.0;

    for (int j = 0; j < z.size(); ++j) {
        for (int i = j; i >= 0; --i) {
            poly(i + 1) -= z(j) * poly(i);
        }
    }

    return poly;
}

rpm::vector<std::complex<double>> poly(const rpm::vector<std::complex<double>>& z) {
    rpm::vector<std::complex<double>> poly(z.size() + 1, 1.0);

    poly[0] = 1.0;

    for (int j = 0; j < z.size(); ++j) {
        for (int i = j; i >= 0; --i) {
            poly[i + 1] -= z[j] * poly[i];
        }
    }

    return poly;
}

rpm::vector<std::array<double, 6>> Analysis::butterworthHighpass(int N, double fc, double fs)
{
    const double Wn = fc / (fs / 2.0);
    const double Wo = tan(Wn * M_PI / 2.0);

    rpm::vector<std::complex<double>> p;

    // Step 1. Get Butterworth analog lowpass prototype.
    for (int i = 2 + N - 1; i <= 3 * N - 1; i += 2) {
        p.push_back(std::polar<double>(1, (M_PI * i) / (2.0 * N)));
    }

    // Step 2. Transform to high pass filter.
    std::complex<double> Sg = 1.0,
                        prodSp = 1.0,
                        prodSz = 1.0;

    rpm::vector<std::complex<double>> Sp(p.size()), Sz(p.size());

    for (int i = 0; i < p.size(); ++i) {
        Sg *= -p[i];
        Sp[i] = Wo / p[i];
        Sz[i] = 0.0;
        prodSp *= (1.0 - Sp[i]);
        prodSz *= (1.0 - Sz[i]);
    }
    Sg = 1.0 / Sg;

    // Step 3. Transform to digital filter.
    rpm::vector<std::complex<double>> P(Sp.size()), Z(Sp.size());
    
    double G = std::real(Sg * prodSz / prodSp);

    for (int i = 0; i < Sp.size(); ++i) {
        P[i] = (1.0 + Sp[i]) / (1.0 - Sp[i]);
        Z[i] = (1.0 + Sz[i]) / (1.0 - Sz[i]);
    }
    
    // Step 6. Convert to SOS.
    
    return zpk2sos(Z, P, G);
}

rpm::vector<std::array<double, 6>> Analysis::butterworthLowpass(int N, double fc, double fs)
{
    const double Wn = fc / (fs / 2.0);
    const double Wo = tan(Wn * M_PI / 2.0);

    rpm::vector<std::complex<double>> p;

    // Step 1. Get Butterworth analog lowpass prototype.
    for (int i = 2 + N - 1; i <= 3 * N - 1; i += 2) {
        p.push_back(std::polar<double>(1, (M_PI * i) / (2.0 * N)));
    }

    // Step 2. Transform to low pass filter.
    std::complex<double> Sg = 1.0,
                        prodSp = 1.0;

    rpm::vector<std::complex<double>> Sp(p.size()), Sz(0);

    for (int i = 0; i < p.size(); ++i) {
        Sg *= Wo;
        Sp[i] = Wo * p[i];
        prodSp *= (1.0 - Sp[i]);
    }

    // Step 3. Transform to digital filter.
    rpm::vector<std::complex<double>> P(Sp.size()), Z(Sp.size(), -1);
   
    double G = std::real(Sg / prodSp);

    for (int i = 0; i < Sp.size(); ++i) {
        P[i] = (1.0 + Sp[i]) / (1.0 - Sp[i]);
    }
    
    // Step 6. Convert to SOS.
    
    return zpk2sos(Z, P, G);
}

