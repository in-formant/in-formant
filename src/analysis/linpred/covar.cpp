#include "linpred.h"

using namespace Analysis::LP;

rpm::vector<double> Covar::solve(const double *data, int length, int lpcOrder, double *pGain)
{
    const int n = length;
    const int m = lpcOrder;

    static rpm::vector<double> b, grc, beta, a, cc;
    b.resize(1 + (m * (m + 1) / 2));
    grc.resize(1 + (m));
    beta.resize(1 + (m));
    a.resize(1 + (m + 1));
    cc.resize(1 + (m + 1));
    double gain;
    int i, j, k;
    std::fill(b.begin(), b.end(), 0.0);
    std::fill(grc.begin(), grc.end(), 0.0);
    std::fill(beta.begin(), beta.end(), 0.0);
    std::fill(a.begin(), a.end(), 0.0);
    std::fill(cc.begin(), cc.end(), 0.0);
   
    // Simulating index-1-based array.
    const double *x = &data[-1];

    gain = 0.0;
    for (i = m + 1; i <= n; ++i) {
        gain += x[i] * x[i];
        cc[1] += x[i] * x[i - 1];
        cc[2] += x[i - 1] * x[i - 1];
    }
    if (gain == 0.0) {
        i = 1;
        gain = 1e-10;
        goto end;
    }

    b[1] = 1.0;
    beta[1] = cc[2];
    a[1] = 1.0;
    a[2] = grc[1] = -cc[1] / cc[2];
    gain += grc[1] * cc[1];

    for (i = 2; i <= m; ++i) {
        double s = 0.0;
        for (j = 1; j <= i; ++j)
            cc[i - j + 2] = cc[i - j + 1]
                            + x[m - i + 1] * x[m - i + j]
                            - x[n - i + 1] * x[n - i + j];
        
        cc[1] = 0.0;
        for (j = m + 1; j <= n; ++j)
            cc[1] += x[j - i] * x[j];

        b[i * (i + 1) / 2] = 1.0;
        for (j = 1; j <= i - 1; ++j) {
            double gam = 0.0;
            if (beta[j] < 0.0)
                goto end;
            else if (beta[j] == 0.0)
                continue;

            for (k = 1; k <= j; ++k)
                gam += cc[k + 1] * b[j * (j - 1) / 2 + k];

            gam /= beta[j];
            for (k = 1; k <= j; ++k)
                b[i * (i - 1) / 2 + k] -= gam * b[j * (j - 1) / 2 + k];
        }

        beta[i] = 0.0;
        for (j = 1; j <= i; ++j)
            beta[i] += cc[j + 1] * b[i * (i - 1) / 2 + j];
        if (beta[i] <= 0.0)
            goto end;

        for (j = 1; j <= i; ++j)
            s += cc[j] * a[j];
        grc[i] = -s / beta[i];

        for (j = 2; j <= i; ++j)
            a[j] += grc[i] * b[i * (i - 1) / 2 + j - 1];
        a[i + 1] = grc[i];
        s = grc[i] * grc[i] * beta[i];
        gain -= s;
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

