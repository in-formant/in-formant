//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_POLYNOMIAL_H
#define SPEECH_ANALYSIS_POLYNOMIAL_H

#include <Eigen/Core>

namespace Polynomial {

    template<typename PolyType, typename ValueType>
    void evaluate(const PolyType & p, const ValueType & x, ValueType & y);

}

#include "Polynomial_impl.h"

#endif //SPEECH_ANALYSIS_POLYNOMIAL_H
