//
// Created by rika on 11/11/2019.
//

#include "../YAAPT.h"

using namespace Eigen;

void YAAPT::medfilt1(const ArrayXd & x, int w, ArrayXd & y)
{
    int w2 = std::floor(w / 2.0);
    w = 2 * w2 + 1;

    const int n = x.size();
    if (n < 1) {
        y.resize(0);
        return;
    }

    ArrayXXd m(w, n + w - 1);
    double x0 = x(0);
    double xl = x(n - 1);

    y.resize(w);

    for (int i = 0; i < w; ++i) {
        m.row(i)(seq(0, i - 1)).setConstant(x0);
        m.row(i)(seq(i, i + n - 1)) = x;
        m.row(i)(seq(i + n, n + w - 2)).setConstant(xl);

        std::vector<double> miv(m.cols());
        std::copy(m.row(i).begin(), m.row(i).end(), miv.begin());

        int k = (n + w - 1) / 2;
        std::nth_element(miv.begin(), miv.begin() + k, miv.end());
        if ((n + w - 1) % 2 == 0) {
            y(i) = miv[k];
        }
        else {
            y(i) = 0.5 * (miv[k] + miv[k - 1]);
        }
    }

    y = y.tail(n);
}