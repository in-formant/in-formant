//
// Created by rika on 10/11/2019.
//

#include <iostream>
#include "Viterbi.h"

using namespace Eigen;

double Viterbi::combinations(int n, int k)
{
    long double result = 1.0;
    if (k > n / 2) k = n - k;
    for (int i = 1; i <= k; ++i) result *= n - i + 1;
    for (int i = 2; i <= k; ++i) result /= i;
    return static_cast<double>(result);
}

void Viterbi::viterbi(
        int numberOfFrames, int maxnCandidates,
        NumberOfCandidatesFn getNumberOfCandidates,
        LocalCostFn getLocalCost,
        TransitionCostFn getTransitionCost,
        PutResultFn putResult,
        void * closure)
{
    ArrayXXd delta(numberOfFrames, maxnCandidates);
    ArrayXXi psi(numberOfFrames, maxnCandidates);
    ArrayXi numberOfCandidates(numberOfFrames);

    for (int iframe = 0; iframe < numberOfFrames; ++iframe) {
        numberOfCandidates(iframe) = getNumberOfCandidates(iframe, closure);
        for (int icand = 0; icand < numberOfCandidates(iframe); ++icand) {
            delta(iframe, icand) = -getLocalCost(iframe, icand, closure);
        }
    }

    for (int iframe = 1; iframe < numberOfFrames; ++iframe) {
        for (int icand2 = 0; icand2 < numberOfCandidates(iframe); ++icand2) {
            double maximum = -1e308;
            int place = 0;
            for (int icand1 = 1; icand1 < numberOfCandidates(iframe - 1); ++icand1) {
                double value = delta(iframe - 1, icand1) + delta(iframe, icand2)
                                - getTransitionCost(iframe, icand1, icand2, closure);
                if (value > maximum) {
                    maximum = value;
                    place = icand1;
                }
            }
            if (place == 0) {
                // cannot compute a track because of weird values.
            }
            delta(iframe, icand2) = maximum;
            psi(iframe, icand2) = place;
        }
    }

    // Find the end of the most probable path.
    int place;
    double maximum = delta(numberOfFrames - 1, place = 0);
    for (int icand = 1; icand < numberOfCandidates(numberOfFrames - 1); ++icand) {
        if (delta(numberOfFrames - 1, icand) > maximum) {
            maximum = delta(numberOfFrames - 1, place = icand);
        }
    }

    // Backtrack.
    for (int iframe = numberOfFrames - 1; iframe >= 0; --iframe) {
        putResult(iframe, place, closure);
        place = psi(iframe, place);
    }
}

struct parm2 {
    int ntrack;
    int ncomb;
    ArrayXXi indices;
    Viterbi::MultiLocalCostFn getLocalCost;
    Viterbi::MultiTransitionCostFn getTransitionCost;
    Viterbi::MultiPutResultFn putResult;
    void * closure;
};

static int getNumberOfCandidates_n(int iframe, void * closure)
{
    auto cl = static_cast<parm2 *>(closure);
    (void) iframe;
    return cl->ncomb;
}

static double getLocalCost_n(int iframe, int jcand, void * closure)
{
    auto cl = static_cast<parm2 *>(closure);

    double localCost = 0.0;
    for (int itrack = 0; itrack < cl->ntrack; ++itrack) {
        localCost += cl->getLocalCost(iframe, cl->indices(jcand, itrack), itrack, cl->closure);
    }
    return localCost;
}

static double getTransitionCost_n(int iframe, int jcand1, int jcand2, void * closure)
{
    auto cl = static_cast<parm2 *>(closure);

    double transitionCost = 0.0;
    for (int itrack = 0; itrack < cl->ntrack; ++itrack) {
        transitionCost += cl->getTransitionCost(iframe,
                cl->indices(jcand1, itrack), cl->indices(jcand2, itrack), itrack, cl->closure);
    }
    return transitionCost;
}

static void putResult_n(int iframe, int jplace, void * closure)
{
    auto cl = static_cast<parm2 *>(closure);

    for (int itrack = 0; itrack < cl->ntrack; ++itrack) {
        cl->putResult(iframe, cl->indices(jplace, itrack), itrack, cl->closure);
    }
}

void Viterbi::viterbiMulti(
        int nframe, int ncand, int ntrack,
        MultiLocalCostFn getLocalCost,
        MultiTransitionCostFn getTransitionCost,
        MultiPutResultFn putResult,
        void * closure)
{
    if (ntrack > ncand) {
        std::cout << "Number of tracks should not excess number of candidates" << std::endl;
        return;
    }

    int ncomb = std::round(combinations(ncand, ntrack));
    if (ncomb > 10'000'000) {
        std::cout << "Unrealistically high number of combinations" << std::endl;
    }

    /*
     * For ncand == 5 and ntrack == 3, parm.indices is going to contain:
     *    0 1 2
     *    0 1 3
     *    0 1 4
     *    0 2 3
     *    0 2 4
     *    0 3 4
     *    1 2 3
     *    1 2 4
     *    1 3 4
     *    2 3 4
     */

    ArrayXXi indices = ArrayXXi::Zero(ncomb, ntrack);
    ArrayXi icand(ntrack);

    for (int itrack = 0; itrack < ntrack; ++itrack) {
        icand(itrack) = itrack; // start out with 0 1 2
    }

    int jcomb = 0;
    for (;;) {
        jcomb++;
        for (int itrack = 0; itrack < ntrack; ++itrack) {
            indices(jcomb-1, itrack) = icand(itrack);
        }
        int itrack = ntrack;
        for (; itrack >= 1; --itrack) {
            if (++icand(itrack-1) <= ncand - (ntrack - (itrack-1))) {
                for (int jtrack = itrack + 1; jtrack <= ntrack; ++jtrack) {
                    icand(jtrack-1) = icand(itrack-1) + (jtrack-1) - (itrack-1);
                }
                break;
            }
        }
        if (itrack == 0) break;
    }

    assert(jcomb == ncomb);

    parm2 parm = {
        .ntrack = ntrack,
        .ncomb = ncomb,
        .indices = indices,
        .getLocalCost = getLocalCost,
        .getTransitionCost = getTransitionCost,
        .putResult = putResult,
        .closure = closure
    };

    Viterbi::viterbi(nframe, ncomb,
                     getNumberOfCandidates_n,
                     getLocalCost_n,
                     getTransitionCost_n,
                     putResult_n,
                     &parm);
}