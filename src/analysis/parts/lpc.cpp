//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "LPC/Frame/LPC_Frame.h"

using namespace Eigen;

void Analyser::analyseLp() {
    lpcFrame.nCoefficients = lpOrder;
    lpFailed = !LPC::frame_burg(x, lpcFrame);
}
