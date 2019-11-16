//
// Created by rika on 12/11/2019.
//

#include <iostream>
#include "Formant.h"
#include "../Math/Viterbi.h"

using namespace Eigen;

struct fparm {
    const Formant::Frames & src;
    Formant::Frames & dest;
    double dfCost, bfCost, octaveJumpCost;
    double refF[5];
};

static double getLocalCost(int iframe, int icand, int itrack, void * closure)
{
    auto cl = static_cast<fparm *>(closure);
    const auto & frame = cl->src[iframe];
    if (icand >= frame.nFormants)
        return 1e30;
    const auto & candidate = frame.formant[icand];
    return cl->dfCost * std::abs(candidate.frequency - cl->refF[itrack])
            + cl->bfCost * candidate.bandwidth / candidate.frequency;
}

static double getTransitionCost(int iframe, int icand1, int icand2, int itrack, void * closure)
{
    auto cl = static_cast<fparm *>(closure);
    const auto & prevFrame = cl->src[iframe - 1];
    const auto & curFrame = cl->src[iframe];
    if (icand1 >= prevFrame.nFormants || icand2 >= curFrame.nFormants)
        return 1e30;
    const double f1 = prevFrame.formant[icand1].frequency;
    const double f2 = curFrame.formant[icand1].frequency;
    return cl->octaveJumpCost * std::abs(std::log2(f1 / f2));
}

static void putResult(int iframe, int place, int itrack, void * closure)
{
    auto cl = static_cast<fparm *>(closure);

    cl->dest[iframe].formant[itrack] = cl->src[iframe].formant[place];
}

void Formant::tracker(
        Frames & src, Frames & dst, int ncand, int ntrack,
        double refF1, double refF2, double refF3, double refF4, double refF5,
        double dfCost, double bfCost, double octaveJumpCost)
{
    int nx = src.size();

    dst.resize(nx);
    for (int iframe = 0; iframe < nx; ++iframe) {
        dst[iframe].formant.resize(ntrack, {0, 0});
        dst[iframe].nFormants = ntrack;
        dst[iframe].intensity = src[iframe].intensity;
    }

    fparm parm = {
        .src = src,
        .dest = dst,
        .dfCost = dfCost / 1000.0,
        .bfCost = bfCost,
        .octaveJumpCost = octaveJumpCost,
        .refF = {
            refF1,
            refF2,
            refF3,
            refF4,
            refF5,
        }
    };

    Viterbi::viterbiMulti(nx, ncand, ntrack, getLocalCost, getTransitionCost, putResult, &parm);
}