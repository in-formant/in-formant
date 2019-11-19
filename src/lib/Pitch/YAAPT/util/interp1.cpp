//
// Created by rika on 10/11/2019.
//

#include <iostream>
#include "../YAAPT.h"

using namespace Eigen;

void YAAPT::interp1(ConstRefXd x, ConstRefXd v, ConstRefXd xq, RefXd vq)
{
    const int nx = x.size();
    const int nq = xq.size();

    for (int i = 0; i < nq; ++i) {
        // pindex is the last index where xq(i) >= x
        int pindex = -1;
        for (int ix = 0; ix < nx; ++ix) {
            if (xq(i) >= x(ix) && (ix == nx - 1 || xq(i) < x(ix + 1))) {
                pindex = ix;
                break;
            }
        }

        // index is the first index where xq(i) <= x
        int index = -1;
        for (int ix = 0; ix < nx; ++ix) {
            if (xq(i) <= x(ix) || ix == nx - 1) {
                index = ix;
                break;
            }
        }

        double slope;
        if (pindex == -1) {
            pindex = index;
            slope = 0;
        }
        else if (index == -1) {
            index = pindex;
            slope = 0;
        }
        else if (pindex == index) {
            slope = 0;
        }
        else if (x(index) == x(pindex)) {
            slope = 0;
        }
        else {
            slope = (xq(i) - x(pindex)) / (x(index) - x(pindex));
        }

        vq(i) = v(pindex) * (1 - slope) + v(index) * slope;
    }
}