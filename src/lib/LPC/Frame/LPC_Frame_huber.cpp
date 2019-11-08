//
// Created by rika on 12/10/2019.
//

#include "../LPC.h"
#include "LPC_Frame.h"
#include "../LPC_huber.h"

using namespace Eigen;
using LPC::Huber::huber_s;

void LPC::frame_huber(const ArrayXd & sound, const LPC::Frame & lpc1, LPC::Frame & lpc2, huber_s & hs)
{
    int p = std::min(lpc1.nCoefficients, lpc2.nCoefficients);

    hs.iter = 0;
    hs.scale = 1e308;
    hs.p = p;

    double scale0;
    do {
        ArrayXd hse = sound;
        LPC::filterInverse(lpc2, hse);

        scale0 = hs.scale;
        VectorXd work = hs.work;
        LPC::Huber::calc_stat(hs.e, &hs.location, hs.wantLocation, &hs.scale, hs.wantScale, hs.k_stdev, hs.tol, 5);

        LPC::Huber::getWeights(hs, hs.e);
        LPC::Huber::getWeightedCovars(hs, sound);

        // Solve C a = [-] c
        LPC::Huber::solveLpc(hs);

        lpc2.a.head(p) = hs.a.head(p);
        hs.iter++;
    }
    while (hs.iter < hs.itermax && std::abs(scale0 - hs.scale) > hs.tol * scale0);
}