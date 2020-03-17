#ifndef EKF_H
#define EKF_H

#include <Eigen/Dense>
#include <deque>
#include "../Formant.h"

namespace EKF
{
    struct State {
        Eigen::VectorXd y;
        bool voiced;
        Eigen::MatrixXd F, Q, R;
        int numF, cepOrder;
        double fs;
        Eigen::VectorXd m_up;
        Eigen::MatrixXd P_up;
    };
    
    void init(State & state, const Eigen::VectorXd & x0);

    void step(State & state);

    Eigen::MatrixXd getH_FBW(
            Eigen::Ref<const Eigen::VectorXd> frmVals,
            Eigen::Ref<const Eigen::VectorXd> bwVals,
            int numFormants,
            int cepOrder,
            double fs);
    
    Eigen::VectorXd fb2cp(
            Eigen::Ref<const Eigen::VectorXd> F,
            Eigen::Ref<const Eigen::VectorXd> BW,
            int numCoeffs,
            double fs);

    Eigen::VectorXd genLPCC(
            const Eigen::ArrayXd & ar,
            int cepOrder);
}

#endif // EKF_H
