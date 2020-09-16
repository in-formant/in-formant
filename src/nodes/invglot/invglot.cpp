#include "invglot.h"

using namespace Nodes;

InvGlot::InvGlot(Analysis::InvglotSolver *solver)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        }),
      mSolver(solver)
{
}

void InvGlot::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioTime>();
   
    Analysis::InvglotResult result = mSolver->solve(in->getConstData(), in->getLength(), in->getSampleRate());

    out->setSampleRate(result.sampleRate);
    out->setLength(result.glotSig.size());
    std::copy(result.glotSig.begin(), result.glotSig.end(), out->getData());
}
