#include "util.h"
#include "aberth.h"
#include <random>

static std::pair<double, double> upperLowerBounds(const rpm::vector<double>& P)
{
    const int degree = static_cast<int>(P.size()) - 1;

    double maxUpper = 0.0;
    double maxLower = 0.0;
    for (int i = 0; i < degree + 1; ++i) {
        const double absPi = std::abs(P[i]);
        if (i >= 1 && absPi > maxUpper)
            maxUpper = absPi;
        if (i < degree && absPi > maxLower)
            maxLower = absPi;
    }

    const double upper = 1.0 + 1.0 / std::abs(P[0]) * maxUpper;
    const double lower = std::abs(P[degree]) / (std::abs(P[degree]) + maxLower);

    return { upper, lower };
}

static rpm::vector<std::complex<double>> initRoots(const rpm::vector<double>& P)
{
    const int degree = static_cast<int>(P.size()) - 1;
    const auto [upper, lower] = upperLowerBounds(P);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> radius(lower, upper);
    static std::uniform_real_distribution<> angle(0, 2 * M_PI);

    rpm::vector<std::complex<double>> roots;
    for (int i = 0; i < degree; ++i) {
        roots.push_back(std::polar(radius(gen), angle(gen)));
    }
    return roots;
}

static rpm::vector<std::complex<double>> evaluatePolyDer(const rpm::vector<double>& P, const std::complex<double>& x, int numberOfDerivatives)
{
    const int degree = static_cast<int>(P.size()) - 1;
    rpm::vector<std::complex<double>> derivatives(numberOfDerivatives + 1);
    numberOfDerivatives = numberOfDerivatives > degree ? degree : numberOfDerivatives;

    derivatives[0] = P[0];

    for (int i = 1; i <= degree; ++i) {
        const int n = numberOfDerivatives < i ? numberOfDerivatives : i;
        for (int j = n; j >= 1; --j) {
            derivatives[j] = derivatives[j] * x + derivatives[j - 1];
        }
        derivatives[0] = derivatives[0] * x + P[i];
    }

    double fact = 1.0;
    for (int j = 2; j <= numberOfDerivatives; ++j) {
        fact *= j;
        derivatives[j] *= fact;
    }
    return derivatives;
}

rpm::vector<std::complex<double>> Analysis::aberthRoots(
                                    const rpm::vector<double>& P)
{
    auto roots = initRoots(P);
    int iteration = 0;
    
    while (true) {
        int valid = 0;
        for (int k = 0; k < roots.size(); ++k) {
            auto y = evaluatePolyDer(P, roots[k], 1);
            auto ratio = y[0] / y[1];

            std::complex<double> sum = 0.0;
            for (int j = 0; j < roots.size(); ++j) {
                if (j != k) {
                    sum += 1.0 / (roots[k] - roots[j]);
                }
            }

            auto offset = ratio / (1.0 - ratio * sum);
            if (std::abs(offset.real()) < 1e-10 && std::abs(offset.imag()) < 1e-10) {
                valid++;
            }
            roots[k] -= offset;
        }
        if (valid == roots.size()) {
            break;
        }
        iteration++;
    }

    return roots;
}
