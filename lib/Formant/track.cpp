#include <cassert>
#include <iostream>
#include "Formant.h"
#include "../Math/Viterbi.h"

using Formant::Frame;

struct fparm {
    const std::deque<Frame> *me;
    std::deque<Frame> *thee;
    double dfCost, bfCost, octaveJumpCost;
    double refF[5];
};

static double getLocalCost(int iframe, int icand, int itrack, void *closure)
{
    const auto parm = (struct fparm *) closure;
    const auto &frm = parm->me->at(iframe-1);
    if (icand > frm.nFormants)
        return 1e30;
    const auto &cand = frm.formant.at(icand-1);

    //assert(cand.bandwidth > 0.0);
    //assert(itrack > 0 && itrack <= 5);

    return parm->dfCost * fabs(cand.frequency - parm->refF[itrack-1])
            + parm->bfCost * 80.0 /*cand.bandwidth*/ / cand.frequency;
}

static double 
getTransitionCost(int iframe, int icand1, int icand2, int itrack, void *closure)
{
    const auto parm = (struct fparm *) closure;
    const auto &prevFrm = parm->me->at(iframe-1 - 1);
    const auto &curFrm = parm->me->at(iframe-1);
    if (icand1 > prevFrm.nFormants || icand2 > curFrm.nFormants)
        return 1e30;
    const double f1 = prevFrm.formant.at(icand1-1).frequency;
    const double f2 = curFrm.formant.at(icand2-1).frequency;
    return parm->octaveJumpCost * fabs(log2(f1 / f2));
}

static void putResult(int iframe, int place, int itrack, void *closure)
{
    auto prm = (struct fparm *) closure;

    //assert(iframe > 0 && iframe <= prm->me->size());
    //assert(itrack > 0 && itrack <= 5);
    //assert(place > 0);
    //assert(place <= prm->me->at(iframe-1).nFormants);

    prm->thee->at(iframe-1).formant.at(itrack-1) = prm->me->at(iframe-1).formant.at(place-1);
}

bool Formant::track(
        std::deque<Frame> &frms,
        int ntrack,
        double refF1, double refF2, double refF3, double refF4, double refF5,
        double dfCost, double bfCost, double octaveJumpCost)
{
    int nFrmMin = std::numeric_limits<int>::max();
    int nFrmMax = 0;
    for (const auto &frm : frms) {
        if (frm.nFormants < nFrmMin) {
            nFrmMin = frm.nFormants;
        }
        if (frm.nFormants > nFrmMax) {
            nFrmMax = frm.nFormants;
        }
    }

    if (nFrmMin < ntrack) {
        // "Number of formants to track (" << ntrack << ") is greater than the minimum number of formants (" << nFrmMin << ")";
        return false;
    }

    int nframe = frms.size();
    std::deque<Frame> outFrms;
    for (int i = 0; i < nframe; ++i) {
        Frame frame;
        frame.formant.resize(ntrack, {.frequency = 0, .bandwidth = 0});
        frame.nFormants = ntrack;
        frame.intensity = frms.at(i).intensity;
        outFrms.push_back(std::move(frame));
    }

    struct fparm parm;
    parm.me = &frms;
    parm.thee = &outFrms;
    parm.dfCost = dfCost / 1000.0;
    parm.bfCost = bfCost;
    parm.octaveJumpCost = octaveJumpCost;
    parm.refF[0] = refF1;
    parm.refF[1] = refF2;
    parm.refF[2] = refF3;
    parm.refF[3] = refF4;
    parm.refF[4] = refF5;

    if (Viterbi::viterbiMulti(
                nframe, nFrmMax, ntrack,
                &getLocalCost, &getTransitionCost, &putResult,
                &parm)) {
        frms = std::move(outFrms);
        return true;
    }

    return false;
}
