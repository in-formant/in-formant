//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_POLYNOMIAL_IMPL_H
#define SPEECH_ANALYSIS_POLYNOMIAL_IMPL_H

#include "Polynomial.h"

template<typename PolyType, typename ValueType>
void Polynomial::evaluate(const PolyType & p, const ValueType & x, ValueType & y) {

    using Scalar = typename ValueType::Scalar;

    int lenx = x.size();
    int lenp = p.size();

    if (lenp < 1) {
        y = ValueType::Zero(lenx);
    }
    else {
        y.setConstant(lenx, p(0));

        for (int k = 1; k < lenp; ++k) {
            y = y * x + Scalar(p(k));
        }
    }

}

#endif //SPEECH_ANALYSIS_POLYNOMIAL_IMPL_H
