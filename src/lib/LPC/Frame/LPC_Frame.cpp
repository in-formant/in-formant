//
// Created by clo on 08/11/2019.
//

#include "LPC_Frame.h"
#include "../LPC.h"
#include "../../Formant/Formant.h"
#include "../../Math/Polynomial.h"

using namespace Eigen;

void LPC::toRoots(const LPC::Frame & lpc, ArrayXcd & r)
{
    const int n = lpc.nCoefficients;

    ArrayXd p(n + 1);
    p(0) = 1.0;
    p.tail(n) = lpc.a;

    Polynomial::roots(p, r);
}

void LPC::toFormantFrame(
        const LPC::Frame & lpc, Formant::Frame & frm,
        double samplingFrequency, double margin)
{
    frm.intensity = lpc.gain;

    if (lpc.nCoefficients == 0) {
        frm.nFormants = 0;
        frm.formant.clear();
    }
    else {
        ArrayXcd r;
        LPC::toRoots(lpc, r);
        Polynomial::fixRootsIntoUnitCircle(r);
        Formant::frameFromRoots(r, frm, samplingFrequency, margin);
    }
}