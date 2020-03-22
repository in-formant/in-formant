#include <Eigen/LU>
#include <numeric>
#include "EKF.h"

using namespace Eigen;

static double clamp(double f, double fs) {
    if (f > fs / 2) {
        f = fs / 2 - (f - fs / 2);
    }
    return std::clamp(abs(f), 60.0, fs / 2 - 60.0);
}

void EKF::step(EKF::State & state)
{
    static MatrixXd P_pred, H, S, K;
    static VectorXd m_pred, y_pred, y_obs;

    const bool voiced = state.voiced;
    const int numF = state.numF;
    const int cepOrder = state.cepOrder;
    const double fs = state.fs;
    auto& y = state.y;
    auto& F = state.F;
    auto Ft = state.F.transpose();
    auto& Q = state.Q;
    auto& R = state.R;

    m_pred = F * state.m_up;
    P_pred = F * state.P_up * Ft + Q;

    for (int i = 0; i < 2 * numF; ++i) {
        if (m_pred(i) > fs / 2) {
            m_pred(i) = fs / 2 - (m_pred(i) - fs / 2);
        }
    }

    auto curFVals = m_pred.head(numF);
    auto curBVals = m_pred.tail(numF);

    // Linearize about m_pred using Taylor expansion.
    H = EKF::getH_FBW(curFVals, curBVals, numF, cepOrder, fs);
    auto Ht = H.transpose();

    if (/*voiced*/true) {
        S = H * P_pred * Ht + R;
        K = (P_pred * Ht) * S.inverse();
    }
    else {
        K.setZero(2 * numF, cepOrder);
    }

    y_pred = fb2cp(curFVals, curBVals, cepOrder, fs);

    y_obs.setZero(cepOrder);
    if (y.size() < cepOrder) {
        y_obs.head(y.size()) = y;
    }
    else {
        y_obs = y.head(cepOrder);
    }

    state.m_up = m_pred + K * (y_obs - y_pred);
    state.P_up = P_pred - K * H * P_pred;

    // Sort the formants by absolute frequency.
    
    std::vector<int> inds(numF);
    std::iota(inds.begin(), inds.end(), 0);
    std::sort(inds.begin(), inds.end(),
            [&](int i, int j) {
                return clamp(state.m_up(i), fs) < clamp(state.m_up(j), fs);
            });

    VectorXd m_ = state.m_up;
    MatrixXd P_ = state.P_up;

    for (int k = 0; k < numF; ++k) {
        m_(k) = clamp(state.m_up(inds[k]), fs);
        m_(numF + k) = clamp(state.m_up(numF + inds[k]), fs);

        P_.row(k) = state.P_up.col(inds[k]);
        P_.row(numF + k) = state.P_up.col(numF + inds[k]);
    }

    state.m_up = std::move(m_);
    state.P_up = std::move(P_);

}
