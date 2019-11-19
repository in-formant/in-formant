//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"

using namespace Eigen;

void Analyser::analyseLp() {
    LPC::Frames lpc = LPC::analyseBurg(
            x,
            lpOrder,
            (CAPTURE_DURATION / 2.0) / 1000.0,
            fs,
            50.0);
    lpcFrame = lpc.d_frames.at(0);
}