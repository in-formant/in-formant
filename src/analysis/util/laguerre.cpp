#include "util.h"
#include "laguerre.h"

std::complex<double> Analysis::laguerreRoot(const std::vector<std::complex<double>>& P, std::complex<double> xk, double accuracy)
{
    constexpr int maxIt = 5000;

    std::vector<std::complex<double>> ys(3);
    ys = evaluatePolynomialDerivatives(P, xk, 2);
  
    const int n = P.size() - 1;

    for (int it = 0; it < maxIt; ++it) {
        if (std::abs(ys[0]) < accuracy)
            return xk;

        auto g = ys[1] / ys[0];
        auto h = g * g - ys[2] / ys[0];
        auto f = std::sqrt(((double) n - 1) * ((double) n * h - g * g));

        std::complex<double> dx;
        if (std::abs(g + f) > std::abs(g - f)) {
            dx = (double) n / (g + f);
        }
        else {
            dx = (double) n / (g - f);
        }
        xk -= dx;

        if (std::abs(dx) < accuracy) {
            return xk;
        }
    
        ys = evaluatePolynomialDerivatives(P, xk, 2);
    }

    return xk;
}

std::vector<std::complex<double>> Analysis::laguerreDeflate(const std::vector<std::complex<double>>& P, const std::complex<double>& root)
{
    const int n = P.size() - 1;

    std::vector<std::complex<double>> Q(n);
    Q[n - 1] = P[n];

    for (int i = n - 2; i >= 0; --i) {
        Q[i] = std::complex<double>(
                std::complex<long double>(P[i + 1])
                    + std::complex<long double>(root) * std::complex<long double>(Q[i + 1]));
    }

    return Q;
}

std::vector<std::complex<double>> Analysis::laguerreSolve(const std::vector<double>& realP)
{
    std::vector<std::complex<double>> P(realP.rbegin(), realP.rend());
    auto Pi = P;

    const int N = P.size() - 1;

    std::vector<std::complex<double>> R(N);

    for (int i = 0; i < N; ++i) {
        R[i] = laguerreRoot(Pi, 0.0, 1e-6);
        Pi = laguerreDeflate(Pi, R[i]);
    }

    for (int i = 0; i < N; ++i) {
        R[i] = laguerreRoot(P, R[i], 1e-12);
    }

    return std::vector<std::complex<double>>(R.begin(), R.end());
}
