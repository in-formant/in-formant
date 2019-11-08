//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_POLYNOMIAL_H
#define SPEECH_ANALYSIS_POLYNOMIAL_H

#include <Eigen/Core>
#include <Eigen/QR>
#include <unsupported/Eigen/Polynomials>

namespace Polynomial {

    template<typename PolyType, typename ValueType>
    inline void evaluate(const PolyType & p, const ValueType & x, ValueType & y) {

        Eigen::Ref<const PolyType> pr = p.reverse();

        if constexpr (std::is_scalar<ValueType>::value) {
            y = poly_eval(pr, x);
        }
        else {
            const int nx = x.size();

            y.resize(nx);
            for (int i = 0; i < nx; ++i) {
                y(i) = poly_eval(pr, x(i));
            }
        }

    }

    template<typename PolyType>
    void roots(const PolyType & p, Eigen::ArrayXcd & z) {
        const int n = p.size();

        Eigen::MatrixXd c(n - 1, n - 1);
        c.setZero();
        c.diagonal(-1).setOnes();
        c.leftCols<1>() = -p.tail(n - 1) / p(0);

        Eigen::ComplexEigenSolver<Eigen::MatrixXd> solver(c);
        z = solver.eigenvalues();
    }

    void fixRootsIntoUnitCircle(Eigen::ArrayXcd & r);

}

#endif //SPEECH_ANALYSIS_POLYNOMIAL_H
