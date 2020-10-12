#include "filter.h"
#include "../../modules/math/constants.h"
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

std::vector<std::complex<float>> poly(const std::vector<std::complex<float>>& z) {
    std::vector<std::complex<float>> poly(z.size() + 1, 1.0f);

    poly[0] = 1.0;

    for (int j = 0; j < z.size(); ++j) {
        for (int i = j; i >= 0; --i) {
            poly[i + 1] -= z[j] * poly[i];
        }
    }

    return poly;
}

std::vector<std::array<float, 6>> Analysis::butterworth(int N, float fc, float fs)
{
    const float Wn = fc / (fs / 2.0f);
    const float Wo = tanf(Wn * M_PI / 2.0f);

    std::vector<std::complex<float>> p;

    // Step 1. Get Butterworth analog lowpass prototype.
    for (int i = 2 + N - 1; i <= 3 * N - 1; i += 2) {
        p.push_back(std::polar<float>(1, (M_PI * i) / (2.0f * N)));
    }

    // Step 2. Transform to high pass filter.
    std::complex<float> Sg = 1.0f,
                        prodSp = 1.0f,
                        prodSz = 1.0f;

    std::vector<std::complex<float>> Sp(p.size()), Sz(p.size());

    for (int i = 0; i < p.size(); ++i) {
        Sg *= -p[i];
        Sp[i] = Wo / p[i];
        Sz[i] = 0.0f;
        prodSp *= (1.0f - Sp[i]);
        prodSz *= (1.0f - Sz[i]);
    }
    Sg = 1.0f / Sg;

    // Step 3. Transform to digital filter.
    std::vector<std::complex<float>> P(Sp.size()), Z(Sp.size());
    
    float G = std::real(Sg * prodSz / prodSp);

    for (int i = 0; i < Sp.size(); ++i) {
        P[i] = (1.0f + Sp[i]) / (1.0f - Sp[i]);
        Z[i] = (1.0f + Sz[i]) / (1.0f - Sz[i]);
    }
    
    // Step 6. Convert to SOS.
    
    return zpk2sos(Z, P, G);
}

