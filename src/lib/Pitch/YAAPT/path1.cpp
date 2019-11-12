//
// Created by rika on 10/11/2019.
//

#include <iostream>
#include "YAAPT.h"

using namespace Eigen;

void YAAPT::path1(const ArrayXXd & local, const Tensor<double, 3> & trans, ArrayXi & path)
{
    const int M = local.rows();
    const int N = local.cols();

    // Initializing several desired matrix.
    ArrayXXi PRED = ArrayXXi::Zero(M, N);
    path.setZero(N);
    ArrayXi p = ArrayXi::Zero(N);
    ArrayXd PCOST = ArrayXd::Zero(M);
    ArrayXd CCOST = ArrayXd::Zero(M);

    // Initializing the previous costs.
    for (int j = 0; j < M; ++j) {
        PCOST(j) = local(j, 0);
    }

    // This loop is the main loop.
    for (int i = 1; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            // Get the lowest cost.
            CCOST(j) = 1e+30;

            for (int k = 0; k < M; ++k) {
                // Deciding the optimal path between two points in two consecutive columns.
                if (PCOST(k) + trans(k, j, i) <= CCOST(j)) {
                    CCOST(j) = PCOST(k) + trans(k, j, i);
                    PRED(j, i) = k;
                }
            }

            if (CCOST(j) >= 1e+30) {
                std::cerr << "path1: CCOST > 1.0e+30, stop." << std::endl;
                return;
            }

            // New cost is calculated with the sum.
            CCOST(j) += local(j, i);
        }

        // Using new costs to update previous costs.
        for (int j = 0; j < M; ++j) {
            PCOST(j) = CCOST(j);
        }

        // Obtaining the points with lowest cost in every column.
        p(i) = 0;
        for (int j = 1; j < M; ++j) {
            if (CCOST(j) <= CCOST(0)) {
                CCOST(0) = CCOST(j);
                p(i) = j;
            }
        }
    }

    // Backtracking.
    path(N - 1) = p(N - 1);
    for (int i = N - 2; i >= 0; --i) {
        path(i) = PRED(p(i + 1), i + 1);
    }
}