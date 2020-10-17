#include "linpred.h"

using namespace Analysis::LP;

std::vector<double> Autocorr::solve(const double *x, int length, int lpcOrder, double *pGain)
{
    const int n = length;
    const int m = lpcOrder;
    
    double gain;

    r.resize(m + 2, 0.0);
    a.resize(m + 2, 0.0);
    rc.resize(m + 1, 0.0);

    int i = 1;

    for (i = 1; i <= m + 1; ++i) {
        for (int j = 1; j <= n - i + 1; ++j) {
            r[i] += x[j - 1] * x[j + i - 2];
        }
    }

    if (r[1] == 0.0) {
        i = 1;
        goto end;
    }

    a[1] = 1.0;
    a[2] = rc[1] = -r[2] / r[1];

    gain = r[1] + r[2] * rc[1];

    for (i = 2; i <= m; ++i) {
        double s = 0.0;
        for (int j = 1; j <= i; ++j) {
            s += r[i - j + 2] * a[j];
        }
        rc[i] = -s / gain;
        for (int j = 2; j <= i / 2 + 1; ++j) {
            double at = a[j] + rc[i] * a[i - j + 2];
            a[i - j + 2] += rc[i] * a[j];
            a[j] = at;
        }
        a[i + 1] = rc[i];

        gain += rc[i] * s;
        if (gain <= 0) {
            goto end;
        }
    }

end:

    std::vector<double> lpc(m);
    
    i--;
    for (int j = 1; j <= i; ++j) {
        lpc[j - 1] = a[j + 1];
    }
    for (int j = i; j < m; ++j) {
        lpc[j] = 0.0f;
    }

    *pGain = gain;

    return lpc;
}
