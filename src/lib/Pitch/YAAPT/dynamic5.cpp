//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

using namespace Eigen;

void YAAPT::dynamic5(
        ConstRefXXd pitchArray, ConstRefXXd meritArray,
        double k1, const Params & prm,
        RefXd finalPitch)
{
    const double F0min = prm.F0min;

    const int numCands = pitchArray.rows();
    const int numFrames = pitchArray.cols();

    // The following weighting factors are used to differentially weight
    // the various types of transitions which can occur, as well as weigh
    // the relative value of transition costs and local costs
    // forming the local cost matrix.
    ArrayXXd localCost = 1 - meritArray;

    // Initialisation for the formation of the transition cost matrices
    Tensor<double, 3> transitionCost(numCands, numCands, numFrames);
    transitionCost.setZero();

    // The transition cost matrix is proportional to frequency differences between successive candidates.
    for (int i = 1; i < numFrames; ++i) {
        for (int j = 0; j < numCands; ++j) {
            for (int k = 0; k < numCands; ++k) {
                double val = abs(pitchArray(j, i) - pitchArray(j, i - 1)) / F0min;
                transitionCost(k, j, i) = 0.05 * val + val * val;
            }
        }
    }

    // Overall balance between local and transition costs.
    transitionCost *= transitionCost.constant(k1);

    // Search the best path.
    ArrayXi path(numFrames);
    path1(localCost, transitionCost, path);

    // Extract the final voiced F0 track which has the lowest cost.
    for (int n = 0; n < numFrames; ++n) {
        finalPitch(n) = pitchArray(path(n), n);
    }
}
