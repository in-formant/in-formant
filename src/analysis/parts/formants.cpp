//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "../../lib/LPC/Frame/LPC_Frame.h"

using namespace Eigen;

void Analyser::analyseFormants() {
    Formant::Frame fFrame{};

    LPC::toFormantFrame(lpcFrame, fFrame, fs);

    formantTrack.pop_front();
    formantTrack.push_back(fFrame);
}