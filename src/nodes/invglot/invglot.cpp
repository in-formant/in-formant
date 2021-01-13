#include "invglot.h"
#include "../../analysis/gci/sigma.h"

using namespace Nodes;

constexpr NodeIOType outputs[] = {
    kNodeIoTypeAudioTime, kNodeIoTypeAudioTime
};

InvGlot::InvGlot(Analysis::InvglotSolver *solver)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 2,
            .outputs = outputs,
        }),
      mSolver(solver)
{
}

void InvGlot::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out1 = outputs[0]->as<IO::AudioTime>();
    auto out2 = outputs[1]->as<IO::AudioTime>();

    Analysis::InvglotResult result = mSolver->solve(in->getConstData(), in->getLength(), in->getSampleRate());

    out1->setSampleRate(result.sampleRate);
    out1->setLength(result.glotSig.size());
    std::copy(result.glotSig.begin(), result.glotSig.end(), out1->getData());
  
    std::vector<double> dg(result.glotSig.size()); 
    for (int i = 1; i < result.glotSig.size(); ++i)
        dg[i] = result.glotSig[i] - result.glotSig[i - 1];
    dg[0] = dg[1];

    //auto gci = SIGMA::analyse(dg, in->getSampleRate());
    out2->setSampleRate(in->getSampleRate());
    out2->setLength(0);//gci.size());
    //std::copy(gci.begin(), gci.end(), out2->getData());
}
