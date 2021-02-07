#include "formants.h"

using namespace Nodes;

constexpr NodeIOType inTypes[] = {
    kNodeIoTypeIIRFilter,
    kNodeIoTypeAudioSpec,
    kNodeIoTypeAudioTime,
};

constexpr NodeIOType outTypes[] = {
    kNodeIoTypeFrequencies,
    kNodeIoTypeFrequencies,
};

FormantTracker::FormantTracker(Analysis::FormantSolver *solver)
    : Node(NodeDescriptor {
            .inputCount = 3,
            .inputs = inTypes,
            .outputCount = 2,
            .outputs = outTypes,
        }),
      mSolver(solver)
{
}

void FormantTracker::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto inFilt = inputs[0]->as<IO::IIRFilter>();
    auto inTime = inputs[2]->as<IO::AudioTime>();
    auto outFs = outputs[0]->as<IO::Frequencies>();
    auto outBs = outputs[1]->as<IO::Frequencies>();
  
    if (auto df = dynamic_cast<Analysis::Formant::DeepFormants *>(mSolver);
            df != nullptr) {
        df->setFrameAudio(
                inTime->getConstData(),
                inTime->getLength(),
                inTime->getSampleRate());
    }

    Analysis::FormantResult result = mSolver->solve(
            inFilt->getFBConstData(),
            inFilt->getFBOrder(),
            inFilt->getSampleRate());

    outFs->setLength(result.formants.size());
    outBs->setLength(result.formants.size());
   
    for (int i = 0; i < result.formants.size(); ++i) { 
        outFs->set(i, result.formants[i].frequency);
        outBs->set(i, result.formants[i].bandwidth);
    }
}
