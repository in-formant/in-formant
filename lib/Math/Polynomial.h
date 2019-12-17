//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_POLYNOMIAL_H
#define SPEECH_ANALYSIS_POLYNOMIAL_H

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <unsupported/Eigen/Polynomials>

template<class T> struct is_complex : std::false_type {};
template<class T> struct is_complex<std::complex<T>> : std::true_type {};

template<typename T> struct is_scalar_or_complex : std::bool_constant<std::is_scalar<T>::value || is_complex<T>::value> {};

namespace Polynomial {

    template<typename PolyType, typename ValueType>
    void evaluate(const PolyType & p, const ValueType & x, ValueType & y)
    {
        if constexpr (is_scalar_or_complex<ValueType>::value) {
            y = poly_eval(p, x);
        }
        else {
            const int nx = x.size();
            y.resize(nx);
            for (int i = 0; i < nx; ++i) {
                y(i) = poly_eval(p, x(i));
            }
        }
    }

    template<typename PolyType, typename ValueType>
    void evaluateWithDerivative(const PolyType & p, const ValueType & x, ValueType & y, ValueType & dy)
    {
        if constexpr (is_scalar_or_complex<ValueType>::value) {
            int lenp = p.size();
            if (lenp < 1) {
                y = 0;
                dy = 0;
            } else {
                y = p(0);
                dy = 0.0;

                for (int k = 1; k < lenp; ++k) {
                    dy = dy * x + y;
                    y = y * x + ValueType(p(k));
                }
            }
        }
        else {
            const int nx = x.size();
            y.resize(nx);
            dy.resize(nx);
            for (int i = 0; i < nx; ++i) {
                evaluateWithDerivative(p, x(i), y(i), dy(i));
            }
        }
    }

    void polishRoot(const Eigen::ArrayXd & p, double * x, int maxit);
    void polishRoot(const Eigen::ArrayXd & p, Eigen::dcomplex * z, int maxit);
    void polishRoots(const Eigen::ArrayXd & p, Eigen::ArrayXcd & r);

    template<typename PolyType>
    void roots(const PolyType & p, Eigen::ArrayXcd & r) {
        const int n = p.size();

        Eigen::MatrixXd c(n - 1, n - 1);
        c.leftCols<1>().setZero();
        c.bottomRows<1>() = -p.head(n - 1);
        c.topRightCorner(n - 2, n - 2).setIdentity();

        r = c.eigenvalues();
        Polynomial::polishRoots(p, r);
    }

    void fixRootsIntoUnitCircle(Eigen::ArrayXcd & r);

}

#endif //SPEECH_ANALYSIS_POLYNOMIAL_H
