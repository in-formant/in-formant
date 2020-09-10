#ifndef NODES_NODE_H
#define NODES_NODE_H

#include "nodeio.h"
#include <vector>

namespace Nodes {

    struct NodeDescriptor {
        int inputCount;
        const NodeIOType *inputs;
        int outputCount;
        const NodeIOType *outputs;
    };

    class Node {
    public:
        Node(NodeDescriptor descriptor);
        virtual ~Node();

        virtual void process(const NodeIO *inputs[], NodeIO *outputs[]) = 0;

        const std::vector<NodeIOType> getInputTypes() const;
        const std::vector<NodeIOType> getOutputTypes() const;

    private:
        std::vector<NodeIOType> mInputTypes;
        std::vector<NodeIOType> mOutputTypes;
    };

    class NodePassthru : public Node {
    public:
        NodePassthru();

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;
    };

}

#include "nodeio_audiotime.h"
#include "nodeio_audiospec.h"
#include "nodeio_frequencies.h"
#include "nodeio_iirfilter.h"

#endif // NODES_NODE_H
