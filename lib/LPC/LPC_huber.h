//
// Created by rika on 12/10/2019.
//

#ifndef SPEECH_ANALYSIS_LPC_HUBER_FRAME_H
#define SPEECH_ANALYSIS_LPC_HUBER_FRAME_H

#include <Eigen/Core>
#include <Eigen/SVD>

namespace LPC::Huber {

    struct huber_s {
        Eigen::ArrayXd e;
        double k_stdev, tol, tol_svd;
        int iter, itermax;
        bool wantLocation, wantScale;
        double location, scale;
        int n, p;
        Eigen::VectorXd w, work, a, c;
        Eigen::MatrixXd covar;
        Eigen::JacobiSVD<Eigen::MatrixXd> svd;
    };

    void init(huber_s & hs, double windowDuration, int p, double samplingFrequency, double location, bool wantLocation);
    void getWeights(huber_s & hs, const Eigen::ArrayXd & e);
    void getWeightedCovars(huber_s & hs, const Eigen::ArrayXd & s);
    void solveLpc(huber_s & hs);

    void calc_stat(const Eigen::ArrayXd & x,
                   double * inout_location, bool wantLocation,
                   double * inout_scale, bool wantScale,
                   double k_stdev, double tol, int maximumNumberOfiterations);

}

#endif //SPEECH_ANALYSIS_LPC_HUBER_FRAME_H
