#include "EKF.h"

using namespace Eigen;

void EKF::init(EKF::State & state, const VectorXd & x0)
{
    const int numF = x0.size() / 2;
    const int cepOrder = state.cepOrder;

    state.numF = numF;

    state.F.setIdentity(2 * numF, 2 * numF);

    state.Q.setZero(2 * numF, 2 * numF);
    state.Q.diagonal().head(numF).setConstant(320 * 320);
    state.Q.diagonal().tail(numF).setConstant(100 * 100);

    state.R.setZero(cepOrder, cepOrder);
    for (int i = 0; i < cepOrder; ++i) {
        state.R(i, i) = 1.0 / (double) (i + 1);
    }

    state.m_up = x0;
    state.P_up = state.Q;
}
