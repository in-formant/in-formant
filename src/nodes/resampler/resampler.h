#ifndef NODES_RESAMPLER_H
#define NODES_RESAMPLER_H

#include "../node/node.h"
#include "../../modules/audio/resampler/resampler.h"

namespace Nodes
{
    class Resampler : public Node {
    public:
        Resampler(int initialInSampleRate, int outSampleRate);

        void setOutputSampleRate(int outSampleRate);

        int getRequiredInputLength(int outLength);

        void process(const NodeIO *inputs, NodeIO *outputs) override;

    private:
        Module::Audio::Resampler mResampler;
    };
}

#endif // NODES_RESAMPLER_H

