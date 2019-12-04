//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"

using namespace Eigen;

void Analyser::analyseLp() {
    LPC::Frames lpc = LPC::analyseBurg(x, lpOrder, fs);
    lpcFrame = lpc.d_frames.at(0);
}