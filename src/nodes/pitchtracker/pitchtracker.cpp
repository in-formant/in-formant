#include "pitchtracker.h"

using namespace Nodes;

PitchTracker::PitchTracker(Analysis::PitchSolver *solver)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeFrequencies,
        }),
      mSolver(solver)
{
}

void PitchTracker::process(const NodeIO *inputs, NodeIO *outputs)
{
    auto in = inputs[0].as<IO::AudioTime>();
    auto out = outputs[0].as<IO::Frequencies>();
   
    Analysis::PitchResult result = mSolver->solve(in->getConstData(), in->getLength(), in->getSampleRate());

    if (result.voiced) {
        out->setLength(1);
        out->set(0, result.pitch);
    }
    else {
        out->setLength(0);
    }
}
