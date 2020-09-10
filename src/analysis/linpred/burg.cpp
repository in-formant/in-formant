#include "linpred.h"

using namespace Analysis::LP;

std::vector<float> Burg::solve(const float *x, int length, int lpcOrder, float *pGain)
{
    int n = length, m = lpcOrder;

    std::vector<double> a(m, 0.0);

    b1.resize(n + 1, 0.0);
    b2.resize(n + 1, 0.0);
    aa.resize(m + 1, 0.0);

    double p = 0.0;
    for (int j = 0; j < n; ++j)
        p += x[j] * x[j];

    double xms = p / static_cast<double>(n);
    if (xms <= 0.0)
        goto end;

    b1[1] = x[0];
    b2[n - 1] = x[n - 1];
    for (int j = 2; j <= n; ++j)
        b1[j] = b2[j - 1] = x[j - 1];

    for (int i = 1; i <= m; ++i) {
        double num = 0.0, denom = 0.0;
        for (int j = 1; j <= n - i; ++j) {
            num += b1[j] * b2[j];
            denom += b1[j] * b1[j] + b2[j] * b2[j];
        }

        if (denom <= 0.0)
            goto end;

        a[i - 1] = (2.0 * num) / denom;

        xms *= 1.0 - a[i - 1] * a[i - 1];

        for (int j = 1; j <= i - 1; ++j)
            a[j - 1] = aa[j] - a[i - 1] * aa[i - j];

        if (i < m) {
            for (int j = 1; j <= i; ++j)
                aa[j] = a[j - 1];
            for (int j = 1; j <= n - i - 1; ++j) {
                b1[j] -= aa[i] * b2[j];
                b2[j] = b2[j + 1] - aa[i] * b1[j + 1];
            }
        }
    }

end:
    std::vector<float> lpc(a.size());
    for (int i = 0; i < a.size(); ++i) {
        lpc[i] = -a[i];
    }

    *pGain = xms;

    return lpc;
}
