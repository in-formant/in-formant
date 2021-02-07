#include "invglot.h"
#include "../wavelet/wavelet.h"
#include <cmath>
#include <chrono>
#include <iostream>

using namespace Analysis::Invglot;
using Analysis::InvglotResult;

static double haarMother(double t)
{
    return (0 <= t && t < 0.5) ? 1 : (0.5 <= t && t < 1) ? -1 : 0;
}

static double haarScaling(double t)
{
    return (0 <= t && t < 1) ? 1 : 0;
}

static double haarFunction(int j, int l, double t)
{
    double pow2j = (1 << j);
    return sqrt(pow2j) * haarMother(pow2j * t - l);
}

AMGIF::AMGIF(int J)
    : iaif(0.98), fft(1 << (J + 1)), Jmax(J)
{
    auto start = std::chrono::steady_clock::now();
    computeC();
    auto end = std::chrono::steady_clock::now();
    std::cout << "Time to compute C : " << (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1'000'000.0) << "s" << std::endl;
}

InvglotResult AMGIF::solve(const double *data, int length, double sampleRate)
{
    auto pFloat = iaif.solve(data, length, sampleRate).glotSig;
    
    rpm::vector<double> d_delta(data, data + length);
    rpm::vector<double> y_epsilon(pFloat.begin(), pFloat.end());

    hwt(d_delta, +1);
    hwt(y_epsilon, +1);

    auto y_bar = y_epsilon;

    do {
        auto y = y_bar;

        Eigen::MatrixXd A(length, length);
        for (int i = 0; i < length; ++i) {
            A.row(i) = Eigen::Map<Eigen::RowVectorXd>(y.data(), length) * C[i];
        }

        
    } while (false);
}

void AMGIF::computeC()
{
    int mu1 = (1 << (Jmax + 1));

    rpm::vector<rpm::vector<double>> basis;
    basis.reserve(mu1);

    rpm::vector<double> func(mu1);

    for (int i = 0; i < mu1; ++i)
        func[i] = haarScaling((double) i / (double) mu1);
    basis.push_back(func);

    for (int j = 0; j <= Jmax; ++j) {
        int nl = (1 << j);
        for (int l = 0; l < nl; ++l) {
            for (int i = 0; i < mu1; ++i)
                func[i] = haarFunction(j, l, (double) i / (double) mu1);
            basis.push_back(func);
        }
    }

    rpm::vector<rpm::vector<rpm::vector<double>>>
        convolutions(mu1, rpm::vector<rpm::vector<double>>(mu1));

    for (int i = 0; i < mu1; ++i) {
        for (int j = 0; j <= i; ++j) {
            rpm::vector<double> conv(mu1);

            // Calculate circular convolution with FFT.

            rpm::vector<std::dcomplex> out(mu1 / 2 + 1);
            for (int k = 0; k < mu1; ++k) {
                fft.input(k) = basis[i][k];
            }
            fft.computeForward();
            for (int k = 0; k < mu1 / 2 + 1; ++k) {
                out[k] = fft.output(k);
            }
            for (int k = 0; k < mu1; ++k) {
                fft.input(k) = basis[j][k];
            }
            fft.computeForward();
            for (int k = 0; k < mu1 / 2 + 1; ++k) {
                fft.output(k) *= out[k];
            }
            fft.computeBackward();
            for (int k = 0; k < mu1; ++k) {
                conv[k] = fft.input(k);
            }

            // Find the Haar wavelet coefficients of the convolution.
            hwt(conv, +1);

            convolutions[i][j] = conv;
            convolutions[j][i] = conv;
        }
    }

    C.resize(mu1, Eigen::MatrixXd::Zero(mu1, mu1));
    
    for (int i = 0; i < mu1; ++i) {
        for (int j = 0; j < mu1; ++j) {
            for (int mu = 0; mu < mu1; ++mu) {
                C[mu](i, j) = convolutions[i][j][mu];
            }
        }
    }
}
