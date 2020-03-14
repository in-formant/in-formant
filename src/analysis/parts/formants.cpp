//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "LPC/Frame/LPC_Frame.h"

using namespace Eigen;

static const Formant::Frame defaultFrame = {
    .nFormants = 5,
    .formant = {{550, 60}, {1650, 60}, {2750, 60}, {3850, 60}, {4950, 60}},
    .intensity = 1.0,
};

void Analyser::analyseFormantLp() {
    if (!lpFailed) {
        LPC::toFormantFrame(lpcFrame, lastFormantFrame, fs);
    }
    else {
        lastFormantFrame = defaultFrame;
    }
}

void Analyser::analyseFormantDeep() {

    bool isVoiced = lastPitchFrame > 0;

}
