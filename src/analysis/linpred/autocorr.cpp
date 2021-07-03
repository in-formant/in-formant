#include "linpred.h"

using namespace Analysis::LP;

rpm::vector<double> Autocorr::solve(const double *data, int length, int lpcOrder, double *pGain)
{
    const int n = length;
    const int m = lpcOrder;

    r.resize(1 + (m + 1));
    a.resize(1 + (m + 1));
    rc.resize(1 + (m));
    double gain;
    int i, j;
    std::fill(r.begin(), r.end(), 0.0);
    std::fill(a.begin(), a.end(), 0.0);
    std::fill(rc.begin(), rc.end(), 0.0);

    j = m + 1;
    while (j--) {
        double d = 0.0;
        for (i = j; i < n; ++i)
            d += data[i] * data[i - j];
        r[j + 1] = d;
    }
    if (r[1] == 0.0) {
        i = 1;
        gain = 1e-10;
        goto end;
    }

    a[1] = 1.0;
    a[2] = rc[1] = -r[2] / r[1];
    gain = r[1] + r[2] * rc[1];

    for (i = 2; i <= m; ++i) {
        double s = 0.0;
        for (j = 1; j <= i; ++j)
            s += r[i - j + 2] * a[j];
        rc[i] = -s / gain;
        for (j = 2; j <= i / 2 + 1; ++j) {
            const double at = a[j] + rc[i] * a[i - j + 2];
            a[i - j + 2] += rc[i] * a[j];
            a[j] = at;
        }
        a[i + 1] = rc[i];
        gain += rc[i] * s;
        if (gain <= 0.0)
            goto end;
    }

end:
    --i;

    if (pGain != nullptr)
        *pGain = gain;
    
    rpm::vector<double> lpc(i);
    for (j = 1; j <= i; ++j)
       lpc[j - 1] = a[j + 1];
    return lpc;
}

