#ifndef NODES_PREEMPH_H
#define NODES_PREEMPH_H

#include "../node/node.h"

namespace Nodes
{
    class PreEmphasis : public Node {
    public:
        PreEmphasis(float frequency);
        
        void setFrequency(float frequency);

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;

    private:
        float mFrequency;
    };
}

#endif // NODES_PREEMPH_H
