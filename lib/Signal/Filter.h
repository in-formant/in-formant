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

    void apply(const Eigen::ArrayXd & b, const Eigen::ArrayXd & x, Eigen::ArrayXd & y);
    void apply(const Eigen::ArrayXd & b, const Eigen::ArrayXd & a, const Eigen::ArrayXd & x, Eigen::ArrayXd & y);

}

#endif //SPEECH_ANALYSIS_FILTER_H
