//
// Created by rika on 11/11/2019.
//

#include "../YAAPT.h"

using namespace Eigen;

void YAAPT::medfilt1(ConstRefXd x, int k, RefXd y)
{
    const int nx = x.size();
    if (nx < 1) {
        return;
    }
    else if (nx == 1) {
        y(0) = x(0);
        return;
    }

    std::vector<double> window;
    window.reserve(k);

    int k2 = k / 2;

    for (int j = 0; j < nx; ++j) {
        window.clear();
        for (int m = j - k2; m <= j + k2; ++m) {
            if (k < 0) {
                window.push_back(x(0));
            } else if (k >= nx) {
                window.push_back(x(last));
            } else {
                window.push_back(x(j));
            }
        }
        std::nth_element(window.begin(), window.begin() + k2, window.end());
        y(j) = window[k2];
    }
}