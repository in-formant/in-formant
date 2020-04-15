//
// Created by rika on 16/11/2019.
//

#include "../Analyser.h"
#include "LPC/Frame/LPC_Frame.h"
#include "Math/Polynomial.h"

using namespace Eigen;

static const Formant::Frame defaultFrame = {
    .nFormants = 5,
    .formant = {{550, 60}, {1650, 60}, {2750, 60}, {3850, 60}, {4950, 60}},
    .intensity = 1.0,
};

void Analyser::analyseFormant() {
    if (lpFailed) {
        lastFormantFrame = defaultFrame;
        return;
    }

    switch (formantMethod) {
        case LP:
            LPC::toFormantFrame(lpcFrame, lastFormantFrame, fs);
            break;
        case KARMA:
            analyseFormantEkf();
            break;
    }

    // Calculate the filter for a higher sampling rate.

    /*const double fsOut = 16000;
    const int Nf = lastFormantFrame.nFormants;
    ArrayXcd z(2 * Nf);

    for (int i = 0; i < Nf; ++i) {

        const double f = std::clamp(lastFormantFrame.formant.at(i).frequency, 100.0, 3800.0);
        const double b = std::clamp(lastFormantFrame.formant.at(i).bandwidth, 80.0, 160.0);
            
        const double r = std::exp(-M_PI * b / fsOut);
        const double phi = 2 * M_PI * f / fsOut;

        z(2 * i) = std::polar(r, phi);
        z(2 * i + 1) = std::polar(r, -phi);
    }

    ArrayXd p;
    Polynomial::fromRoots(z, p);*/
}

void Analyser::analyseFormantEkf() {

    const int numF = ekfState.numF;
   
    ekfState.voiced = (lastPitchFrame != 0);
    ekfState.fs = this->fs;

    EKF::step(ekfState);
   
    Formant::Frame frm;

    frm.nFormants = numF;
    frm.formant.resize(numF);

    for (int i = 0; i < numF; ++i) {
        frm.formant[i].frequency = ekfState.m_up(i);
        frm.formant[i].bandwidth = ekfState.m_up(numF + i);
    }

    Formant::sort(frm);

    lastFormantFrame = std::move(frm);

}
