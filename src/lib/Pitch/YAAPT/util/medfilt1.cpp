//
// Created by rika on 11/11/2019.
//

#include "../YAAPT.h"

using namespace Eigen;

void YAAPT::medfilt1(const ArrayXd & x, int k, ArrayXd & y)
{
    const int nx = x.size();
    if (nx < 1) {
        y.resize(0);
        return;
    }
    if (nx == 1) {
        y = x;
        return;
    }

    int k2 = (k - 1) / 2;
    ArrayXXd m;
    m.setZero(nx, k);

    m.col(k2) = x;
    for (int i = 0; i < k2; ++i) {
        int j = k2 - i;
        m(seq(j, last), i) = x(seq(0, (nx - j - 1)));
        m(seq(0, j - 1), i) = x(0);
        m(seq(0, (nx - j - 1)), (k - (i + 1) - 1)) = x(seq(j, last));
        m(seq((nx - j - 1), last), (k - (i + 1) - 1)) = x(last);
    }

    y.resize(nx);
    for (int i = 0; i < nx; ++i) {
        ArrayXd row = m.row(i);
        std::nth_element(row.begin(), row.begin() + k2, row.begin());
        y(i++) = row(k2);
    }
}