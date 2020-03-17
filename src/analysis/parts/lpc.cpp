//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "LPC/Frame/LPC_Frame.h"
#include "Formant/EKF/EKF.h"

using namespace Eigen;

void Analyser::analyseLp() {
    lpcFrame.nCoefficients = lpOrder;
    lpFailed = !LPC::frame_burg(x, lpcFrame);

    if (!lpFailed) {
        ekfState.y = EKF::genLPCC(lpcFrame.a, cepOrder);
    }
    else {
        ekfState.y.setZero(ekfState.cepOrder);
    }
}
