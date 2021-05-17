#include "linpred.h"
#include <algorithm>

using namespace Analysis::LP;

static double vecBurgBuffered(
        double *lpc,
        const int m,
        const double *data,
        const int n)
{
    static rpm::vector<double> b1, b2, aa;
    b1.resize(1 + (n));
    b2.resize(1 + (n));
    aa.resize(1 + (m));
    int i, j;
    std::fill(b1.begin(), b1.end(), 0.0);
    std::fill(b2.begin(), b2.end(), 0.0);
    std::fill(aa.begin(), aa.end(), 0.0);

    // Simulating index-1-based array.
    double *a = &lpc[-1];
    const double *x = &data[-1];
    
    double p = 0.0;
    for (j = 1; j <= n; ++j)
        p += x[j] * x[j];

    double xms = p / n;
    if (xms <= 0.0) {
        return xms;
    }

    b1[1] = x[1];
    b2[n - 1] = x[n];
    for (j = 2; j <= n - 1; ++j)
        b1[j] = b2[j - 1] = x[j];

    for (i = 1; i <= m; ++i) {
        double num = 0.0, denum = 0.0;
        for (j = 1; j <= n - i; ++j) {
            num += b1[j] * b2[j];
            denum += b1[j] * b1[j] + b2[j] * b2[j];
        }

        if (denum <= 0.0)
            return 0.0;

        a[i] = 2.0 * num / denum;

        xms *= 1.0 - a[i] * a[i];

        for (j = 1; j <= i - 1; ++j)
            a[j] = aa[j] - a[i] * aa[i - j];

        if (i < m) {
            for (j = 1; j <= i; ++j)
                aa[j] = a[j];
            for (j = 1; j <= n - i - 1; ++j) {
                b1[j] -= aa[i] * b2[j];
                b2[j] = b2[j + 1] - aa[i] * b1[j + 1];
            }
        }
    }
   
    return xms; 
}

rpm::vector<double> Burg::solve(const double *x, int length, int lpcOrder, double *pGain)
{
    const int n = length;
    const int m = lpcOrder;
    rpm::vector<double> lpc(m);
    double gain = vecBurgBuffered(lpc.data(), m, x, n);
    if (gain <= 0.0) {
        lpc.resize(0);
        gain = 1e-10;
    }
    gain *= n;
    for (auto& x : lpc) {
        x = -x;
    }
    if (pGain != nullptr)
        *pGain = gain;
    return lpc;
}
