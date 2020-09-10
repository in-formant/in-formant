#ifndef NODES_FORMANTS_H
#define NODES_FORMANTS_H

#include "../node/node.h"
#include "../../analysis/formant/formant.h"

namespace Nodes
{
    class FormantTracker : public Node {
    public:
        FormantTracker(Analysis::FormantSolver *solver);

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;

    private:
        Analysis::FormantSolver *mSolver;
    };
}

#endif // NODES_FORMANTS_H
