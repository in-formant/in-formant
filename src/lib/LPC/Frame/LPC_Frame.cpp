//
// Created by clo on 08/11/2019.
//

#include "LPC_Frame.h"
#include "../LPC.h"
#include "../../Formant/Formant.h"
#include "../../Math/Polynomial.h"

using namespace Eigen;

void LPC::toFormantFrame(
        const LPC::Frame & lpc, Formant::Frame & frm,
        double samplingFrequency)
{
    frm.intensity = lpc.gain;

    if (lpc.nCoefficients == 0) {
        frm.nFormants = 0;
        frm.formant.clear();
    }
    else {
        ArrayXcd r;

        ArrayXd p(lpc.nCoefficients + 1);
        p(0) = 1.0;
        p.tail(lpc.nCoefficients) = lpc.a;

        Polynomial::roots(p, r);
        Polynomial::fixRootsIntoUnitCircle(r);
        Formant::frameFromRoots(p, r, frm, samplingFrequency);
    }
}