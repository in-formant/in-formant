#ifndef NODES_PITCHTRACKER_H
#define NODES_PITCHTRACKER_H

#include "../node/node.h"
#include "../../analysis/pitch/pitch.h"

namespace Nodes
{
    class PitchTracker : public Node {
    public:
        PitchTracker(Analysis::PitchSolver *solver);

        void process(const NodeIO *inputs, NodeIO *outputs) override;

    private:
        Analysis::PitchSolver *mSolver;
    };
}

#endif // NODES_PITCHTRACKER_H
