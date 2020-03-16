//
// Created by rika on 10/11/2019.
//

#ifndef SPEECH_ANALYSIS_VITERBI_H
#define SPEECH_ANALYSIS_VITERBI_H

#include <Eigen/Core>

namespace Viterbi
{
    double combinations(int n, int k);

    using NumberOfCandidatesFn = int (*)(int iframe, void * closure);
    using LocalCostFn = double (*)(int iframe, int icand, void * closure);
    using TransitionCostFn = double (*)(int iframe, int icand1, int icand2, void * closure);
    using PutResultFn = void (*)(int iframe, int place, void * closure);

    bool viterbi(int numberOfFrames, int maxnCandidates,
                 NumberOfCandidatesFn getNumberOfCandidates,
                 LocalCostFn getLocalCost,
                 TransitionCostFn getTransitionCost,
                 PutResultFn putResult,
                 void * closure);

    using MultiLocalCostFn = double (*)(int iframe, int icand, int itrack, void * closure);
    using MultiTransitionCostFn = double (*)(int iframe, int icand1, int icand2, int itrack, void * closure);
    using MultiPutResultFn = void (*)(int iframe, int place, int itrack, void * closure);

    bool viterbiMulti(int nframe, int ncand, int ntrack,
                      MultiLocalCostFn getLocalCost,
                      MultiTransitionCostFn getTransitionCost,
                      MultiPutResultFn putResult,
                      void * closure);

}

#endif //SPEECH_ANALYSIS_VITERBI_H
