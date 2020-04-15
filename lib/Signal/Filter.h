//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_FILTER_H
#define SPEECH_ANALYSIS_FILTER_H

#include <Eigen/Core>

namespace Filter {

    void responseFIR(const Eigen::ArrayXd & a, const Eigen::ArrayXd & f, double fs, Eigen::ArrayXcd & h);

    void responseIIR(const Eigen::ArrayXd & b, const Eigen::ArrayXd & a, const Eigen::ArrayXd & f, double fs, Eigen::ArrayXcd & h);

    void preEmphasis(Eigen::ArrayXd & x, double samplingFrequency, double preEmphasisFrequency);

    template<typename D1, typename D2, typename D3>
    void apply(const Eigen::DenseBase<D1> & b, const Eigen::DenseBase<D2> & x, Eigen::DenseBase<D3> & y)
    {
        y.resize(x.size());
        for (int n = 0; n < x.size(); ++n) {
            y(n) = 0.0;
            for (int i = 0; i < b.size() && n - i >= 0 && n - i < x.size(); ++i) {
                y(n) += b(i) * x(n - i);
            }
        }
    }

    template<typename D1, typename D2, typename D3, typename D4>
    void apply(const Eigen::DenseBase<D1> & b, const Eigen::DenseBase<D2> & a, const Eigen::DenseBase<D3> & x, Eigen::DenseBase<D4> & y)
    {
        y.resize(x.size());
        for (int n = 0; n < x.size(); ++n) {
            y(n) = 0.0;
            for (int i = 0; i < b.size() && n - i >= 0 && n - i < x.size(); ++i) {
                y(n) += b(i) * x(n - i);
            }
            for (int j = 1; j < a.size() && n - j >= 0 && n - j < x.size(); ++j) {
                y(n) -= (a(j) * y(n - j)) / a(0);
            }
        }
    }

}

#endif //SPEECH_ANALYSIS_FILTER_H
