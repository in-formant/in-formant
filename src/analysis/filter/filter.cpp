#include "filter.h"

rpm::vector<double> Analysis::filter(const rpm::vector<double>& a, const rpm::vector<double>& x)
{
    const int nx = x.size();
    const int na = a.size();

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
    const int nx = x.size();
    const int nb = b.size();
    const int na = a.size();

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


