#include "filter.h"

std::vector<float> Analysis::filter(const std::vector<float>& a, const std::vector<float>& x)
{
    const int nx = x.size();
    const int na = a.size();

    std::vector<float> y(nx);

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

std::vector<float> Analysis::filter(const std::vector<float>& b, const std::vector<float>& a, const std::vector<float>& x)
{
    const int nx = x.size();
    const int nb = b.size();
    const int na = a.size();

    std::vector<float> y(nx);

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


