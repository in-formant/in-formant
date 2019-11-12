//
// Created by rika on 10/11/2019.
//

//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

using namespace Eigen;

void YAAPT::dynamic(
        const ArrayXXd & pitch, const ArrayXXd & merit,
        const ArrayXd & energy, const Params & prm,
        ArrayXd &finalPitch)
{
    const int numCands = pitch.rows();
    const int numFrames = pitch.cols();

    ArrayXd bestPitch = pitch.row(numCands - 2);
    // mean(bestPitch(bestPitch>0))
    double meanPitch = 0.0;
    int numPosPitch = 0;
    for (double f : bestPitch)
        if (f > 0)
            meanPitch += f;
    if (numPosPitch > 0)
        meanPitch /= static_cast<double>(numPosPitch);

    // The following weighting factors are used to differentially weight
    // the various types of transitions which can occur, as well as weigh
    // the relative value of transition costs and local costs
    // forming the local cost matrix.
    double dp_w1 = prm.dp_w1;
    double dp_w2 = prm.dp_w2;
    double dp_w3 = prm.dp_w3;
    double dp_w4 = prm.dp_w4;

    // Forming the local cost matrix.
    ArrayXXd localCost = 1 - merit;

    // Initialisation for the formation of the transition cost matrices
    Tensor<double, 3> transitionCost(numCands, numCands, numFrames);
    transitionCost.setConstant(1);

    // The transition cost matrix is proportional to frequency differences between successive candidates.
    for (int i = 1; i < numFrames; ++i) {
        for (int j = 0; j < numCands; ++j) {
            for (int k = 0; k < numCands; ++k) {
                // Both candidates voiced
                if (pitch(j, i) > 0 && pitch(k, i - 1) > 0) {
                    transitionCost(k, j, i) = dp_w1 * (abs(pitch(j, i) - pitch(k, i - 1)) / meanPitch);
                }

                // One candidate is unvoiced
                if ((pitch(j, i) == 0 && pitch(k, i - 1) > 0)
                        || (pitch(j, i) > 0 && pitch(k, i - 1) == 0)) {
                    double benefit = std::min(1.0, std::abs(energy(i - 1) - energy(i)));
                    transitionCost(k, j, i) = dp_w2 * (1 - benefit);
                }

                // Both candidates are unvoiced
                if (pitch(j, i) == 0 && pitch(k, i - 1) == 0) {
                    transitionCost(j, j, i) = dp_w3;
                }
            }
        }
    }

    // Overall balance between local and transition costs
    transitionCost /= transitionCost.constant(dp_w4);

    // Find the minimum cost path through pitch using the local and transition costs.
    ArrayXi path;
    path1(localCost, transitionCost, path);

    // Extracting the pitch, using path.
    finalPitch.setZero(numFrames);
    for (int n = 0; n < numFrames; ++n) {
        finalPitch(n) = pitch(path(n), n);
    }

}