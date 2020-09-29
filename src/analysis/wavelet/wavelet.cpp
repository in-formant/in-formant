#include "wavelet.h"
#include "../../modules/math/constants.h"

void Analysis::hwt(std::vector<double>& x, int dir)
{
    const int n = x.size();

    std::vector<double> y(n, 0.0);

    if (dir == +1) {
        int k = 1;
        while (k * 2 <= n) {
            k *= 2;
        }

        while (1 < k) {
            k /= 2;
            for (int i = 0; i < k; ++i) {
                y[i]     = (x[2 * i] + x[2 * i + 1]) / M_SQRT2;
                y[i + k] = (x[2 * i] - x[2 * i + 1]) / M_SQRT2;
            }
            for (int i = 0; i < k * 2; ++i) {
                x[i] = y[i];
            }
        }
    }
    else if (dir == -1) {
        int k = 1;
        while (k * 2 <= n) {
            for (int i = 0; i < k; ++i) {
                y[2 * i]     = (x[i] + x[i + k]) / M_SQRT2;
                y[2 * i + 1] = (x[i] - x[i + k]) / M_SQRT2;
            }
            for (int i = 0; i < k * 2; ++i) {
                x[i] = y[i];
            }
            k *= 2;
        }
    }
}
