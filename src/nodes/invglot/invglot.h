#ifndef NODES_INVGLOT_H
#define NODES_INVGLOT_H

#include "../node/node.h"
#include "../../analysis/invglot/invglot.h"

namespace Nodes
{
    class InvGlot : public Node {
    public:
        InvGlot(Analysis::InvglotSolver *solver);

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;

    private:
        Analysis::InvglotSolver *mSolver;
    };
}

#endif // NODES_INVGLOT_H
