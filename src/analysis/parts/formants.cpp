//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "../../lib/LPC/Frame/LPC_Frame.h"

using namespace Eigen;

void Analyser::analyseFormantLp() {
    Formant::Frame fFrame{};

    LPC::toFormantFrame(lpcFrame, fFrame, fs);

    formantTrack.pop_front();
    formantTrack.push_back(fFrame);
}

void Analyser::analyseFormantDeep() {

    bool isVoiced = pitchTrack.back() > 0;

}