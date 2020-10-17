#include "filter.h"

std::vector<double> Analysis::filter(const std::vector<double>& a, const std::vector<double>& x)
{
    const int nx = x.size();
    const int na = a.size();

    std::vector<double> y(nx);

    for (int i = 0; i < nx; ++i) {
        y[i] = 0.0f;
        for (int j = 0; j < na; ++j) {
            if (i - j >= 0 && i - j < nx) {
                y[i] += a[j] * x[i - j];
            }
        }
    }

    return y;
}

std::vector<double> Analysis::filter(const std::vector<double>& b, const std::vector<double>& a, const std::vector<double>& x)
{
    const int nx = x.size();
    const int nb = b.size();
    const int na = a.size();

    std::vector<double> y(nx);

    for (int i = 0; i < nx; ++i) {
        y[i] = 0.0f;
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


