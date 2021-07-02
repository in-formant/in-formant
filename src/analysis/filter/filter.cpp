#include "filter.h"

rpm::vector<double> Analysis::filter(const rpm::vector<double>& a, const rpm::vector<double>& x)
{
    const int nx = (int) x.size();
    const int na = (int) a.size();

    rpm::vector<double> y(nx);

    for (int i = 0; i < nx; ++i) {
        y[i] = 0.0;
        for (int j = 0; j < na; ++j) {
            if (i - j >= 0 && i - j < nx) {
                y[i] += a[j] * x[i - j];
            }
        }
    }

    return y;
}

rpm::vector<double> Analysis::filter(const rpm::vector<double>& b, const rpm::vector<double>& a, const rpm::vector<double>& x)
{
    const int nx = (int) x.size();
    const int nb = (int) b.size();
    const int na = (int) a.size();

    rpm::vector<double> y(nx);

    for (int i = 0; i < nx; ++i) {
        y[i] = 0.0;
        for (int j = 0; j < nb; ++j) {
            if (i - j >= 0 && i - j < nx) {
                y[i] += b[j] * x[i - j];
            }
        }
        for (int j = 1; j < na; ++j) {
            if (i - j >= 0 && i - j < nx) {
                y[i] -= a[j] * y[i - j];
            }
        }
    }

    return y;
}

static inline double G(double x, int L, double alpha)
{
    const int N = L - 1;
    const double k = (x - N / 2.0) / (2 * L * alpha);
    return exp(-(k * k));
}

rpm::vector<double> Analysis::gaussianWindow(int L, double alpha)
{
    const double Gmh = G(-0.5, L, alpha);
    const double GmhpLpGmhmL = G(-0.5 + L, L, alpha) - G(-0.5 - L, L, alpha);

    rpm::vector<double> win(L);
    for (int n = 0; n < L; ++n) {
        win[n] = G(n, L, alpha) - (Gmh * (G(n + L, L, alpha) + G(n - L, L, alpha))) / GmhpLpGmhmL;
    }
    return win;
}

rpm::vector<double> Analysis::blackmanHarrisWindow(int L)
{
    constexpr double a0 = 0.35875;
    constexpr double a1 = 0.48829;
    constexpr double a2 = 0.14128;
    constexpr double a3 = 0.01168;
    rpm::vector<double> w(L);
    for (int j = 0; j < L; ++j) { 
        w[j] = a0 - a1 * cos((2.0 * M_PI * j) / (L - 1))
                  + a2 * cos((4.0 * M_PI * j) / (L - 1))
                  - a3 * cos((6.0 * M_PI * j) / (L - 1));
    }
    return w;
}
