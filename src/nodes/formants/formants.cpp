#include "formants.h"

using namespace Nodes;

constexpr NodeIOType inTypes[] = {
    kNodeIoTypeIIRFilter,
    kNodeIoTypeAudioSpec,
};

constexpr NodeIOType outTypes[] = {
    kNodeIoTypeFrequencies,
    kNodeIoTypeFrequencies,
};

FormantTracker::FormantTracker(Analysis::FormantSolver *solver)
    : Node(NodeDescriptor {
            .inputCount = 2,
            .inputs = inTypes,
            .outputCount = 2,
            .outputs = outTypes,
        }),
      mSolver(solver)
{
}

void FormantTracker::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::IIRFilter>();
    auto outFs = outputs[0]->as<IO::Frequencies>();
    auto outBs = outputs[1]->as<IO::Frequencies>();
   
    Analysis::FormantResult result = mSolver->solve(
            in->getFFConstData(),
            in->getFFOrder(),
            in->getSampleRate());

    outFs->setLength(result.formants.size());
    outBs->setLength(result.formants.size());
   
    for (int i = 0; i < result.formants.size(); ++i) { 
        outFs->set(i, result.formants[i].frequency);
        outBs->set(i, result.formants[i].bandwidth);
    }
}
