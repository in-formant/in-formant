//
// Created by clo on 03/12/2019.
//

#ifndef SPEECH_ANALYSIS_FEATURES_H
#define SPEECH_ANALYSIS_FEATURES_H

#include <Eigen/Core>

struct feat_spec {
    Eigen::ArrayXd pxx, fx;
};

void buildFeatureRow(const Eigen::ArrayXd & x, std::vector<double> & v);

void featureSpec(const Eigen::ArrayXd & x, Eigen::ArrayXd & v);
void featureArspec(const Eigen::ArrayXd & x, int order, Eigen::ArrayXd & v);

void dct(const Eigen::ArrayXd & in, Eigen::ArrayXd & out);

void periodogram(const Eigen::ArrayXd & x, feat_spec & r, int nfft);
void arspec(const Eigen::ArrayXd & x, feat_spec & r, int order, int nfft);

void lpc_lev(const Eigen::ArrayXd & x, int order, Eigen::ArrayXd & a, double & e);

inline int nextpow2(int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

#endif //SPEECH_ANALYSIS_FEATURES_H
