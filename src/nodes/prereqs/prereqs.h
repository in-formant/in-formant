#ifndef NODES_PREREQS_H
#define NODES_PREREQS_H

#include "../node/node.h"
#include "../../modules/audio/buffer/buffer.h"

namespace Nodes
{
    class Prereqs : public Node {
    public:
        Prereqs(Module::Audio::Buffer *buffer, int outDurationInMs, int minOutLength);

        void setOutputDuration(int outDuration);
        void setMinimumOutputLength(int minOutLength);

        void process(const NodeIO *inputs, NodeIO *outputs) override;

    private:
        Module::Audio::Buffer *mBuffer;
        int mOutDuration;
        int mMinOutLength;
    };
}

#endif // NODES_PREREQS_H
