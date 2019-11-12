//
// Created by rika on 10/11/2019.
//

#include <iostream>
#include "../YAAPT.h"

using namespace Eigen;

void YAAPT::interp1(const ArrayXd & x, const ArrayXd & v,
                    const ArrayXd & xq, ArrayXd & vq)
{
    const int nx = x.size();
    const int nq = xq.size();

    vq.resize(nq);
    for (int i = 0; i < nq; ++i) {
        int pindex = -1, index = -1;
        // pindex is the last index where xq(i) >= x
        // index is the first index where xq(i) <= x
        int idx = 0;
        while (idx < nx) { //FIXME
            if (xq(i) >= x(idx))
                pindex = idx;
            if (index < 0 && xq(i) <= x(idx))
                index = idx;
            if (index >= 0 && xq(i) < x(idx))
                break;
            idx++;
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