#ifndef NODES_TAIL_H
#define NODES_TAIL_H

#include "../node/node.h"

namespace Nodes
{
    class Tail : public Node {
    public:
        Tail(int outDurationInMs);

        void setOutputDuration(int outDuration);

        void process(const NodeIO *inputs, NodeIO *outputs) override;

    private:
        int mOutDuration;
    };
}

#endif // NODES_TAIL_H

