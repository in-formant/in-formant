//
// Created by clo on 08/11/2019.
//

#include "Polynomial.h"

using namespace Eigen;

void Polynomial::fixRootsIntoUnitCircle(ArrayXcd & r)
{
    for (auto & v : r) {
        if (std::abs(v) > 1.0) {
            v = 1.0 / std::conj(v);
        }
    }
}
