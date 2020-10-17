#ifndef NODES_LINPRED_H
#define NODES_LINPRED_H

#include "../node/node.h"
#include "../../analysis/linpred/linpred.h"
#include "../../analysis/fft/fft.h"

namespace Nodes
{
    class LinPred : public Node {
    public:
        LinPred(Analysis::LinpredSolver *solver, int order);

        void setOrder(int order);

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;

    private:
        Analysis::LinpredSolver *mSolver;
        Analysis::ReReFFT mFFT;
        int mOrder;

        std::vector<double> mLastSpec;
    };
}

#endif // NODES_LINPRED_H
