#include "synthesis.h"
#include "../modules/math/constants.h"
#include <cmath>

static std::pair<std::vector<double>, std::vector<double>>
    calculateFormantFilter(const float frequency, const float bandwidth, const float sampleRate)
{
    const double r = exp(-M_PI * bandwidth / sampleRate);
    const double wc = 2.0 * M_PI * frequency / sampleRate;

    const double b0 = (1.0 - r) * sqrt(1.0 - 2.0 * r * cos(2.0 * wc) + (r * r));

    const double a1 = -2.0 * r * cos(wc);
    const double a2 = r * r;

    return {{b0}, {1.0, a1, a2}};
}

static std::vector<double> conv(const std::vector<double>& x, const std::vector<double>& y)
{
    const int lx = x.size();
    const int ly = y.size();

    const int lw = lx + ly - 1;

    std::vector<double> w(lw, 0.0);
    
    for (int k = 0; k < lw; ++k) {
        int i = k;
        for (int j = 0; j < ly; ++j) {
            if (i >= 0 && i < lx) {
                w[k] += x[i] * y[j];
            }
            i--;
        }
    }

    return w;
}

std::pair<std::vector<float>, std::vector<float>>
Synthesis::generateVTFilter(const Nodes::IO::Frequencies *frequencies,
                            const Nodes::IO::Frequencies *bandwidths,
                            const float Fs)
{
    const double gain = 2;

    std::vector<double> B {gain};
    std::vector<double> A {1.0};

    for (int i = 0; i < frequencies->getLength(); ++i) {
        auto [Bs, As] = calculateFormantFilter(
                            frequencies->get(i),
                            bandwidths->get(i),
                            Fs);
    
        B = conv(B, Bs);
        A = conv(A, As);
    }

    return {std::vector<float>(B.begin(), B.end()),
            std::vector<float>(A.begin(), A.end())};
}
