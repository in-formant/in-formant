//
// Created by rika on 22/11/2019.
//

#include "MPM.h"

std::vector<int> MPM::peakPicking(Ref<const ArrayXd> x) {
    std::vector<int> maxPositions;

    int pos = 0;
    int currentMaxPos = 0;
    int size = x.size();

    while (pos < (size - 1) / 3 && x(pos) > 0)
        pos++;
    while (pos < size - 1 && x(pos) <= 0)
        pos++;

    if (pos == 0)
        pos = 1;

    while (pos < size - 1) {
        if (x(pos) > x(pos - 1) && x(pos) >= x(pos + 1)
            && (currentMaxPos == 0 || x(pos) > x(currentMaxPos))) {
            currentMaxPos = pos;
        }
        pos++;
        if (pos < size - 1 && x(pos) <= 0) {
            if (currentMaxPos > 0) {
                maxPositions.push_back(currentMaxPos);
                currentMaxPos = 0;
            }
            while (pos < size - 1 && x(pos) <= 0)
                pos++;
        }
    }

    if (currentMaxPos > 0) {
        maxPositions.push_back(currentMaxPos);
    }

    return maxPositions;
}