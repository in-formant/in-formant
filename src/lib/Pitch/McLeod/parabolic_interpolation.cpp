//
// Created by rika on 22/11/2019.
//

#include "MPM.h"

std::pair<double, double> MPM::parabolicInterpolation(Ref<const ArrayXd> array, int x) {
    int x_adjusted;
    double x_ = x;

    if (x < 1) {
        x_adjusted = (array(x) <= array(x + 1)) ? x : x + 1;
    }
    else if (x > int(array.size()) - 1) {
        x_adjusted = (array(x) <= array(x - 1)) ? x : x - 1;
    }
    else {
        double den = array(x + 1) + array(x - 1) - 2 * array(x);
        double delta = array(x - 1) - array(x + 1);
        return (den == 0) ? std::make_pair(x_, array(x))
                          : std::make_pair(x_ + delta / (2 * den),
                                  array(x) - delta * delta / (8 * den));
    }
    return std::make_pair(x_adjusted, array(x_adjusted));
}