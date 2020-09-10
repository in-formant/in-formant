#include "linpred.h"
#include <iostream>

using namespace Nodes;

constexpr NodeIOType outTypes[] = {
    kNodeIoTypeIIRFilter,
    kNodeIoTypeAudioSpec,
};

LinPred::LinPred(Analysis::LinpredSolver *solver, int order)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = outTypes,
        }),
      mSolver(solver),
      mFFT(512),
      mOrder(order)
{
}

void LinPred::setOrder(int order)
{
    mOrder = order;
}

void LinPred::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::IIRFilter>();
    auto outSpec = outputs[1]->as<IO::AudioSpec>();

    int sampleRate = in->getSampleRate();

    float gain;

    std::vector<float> lpc = mSolver->solve(
            in->getConstData(),
            in->getLength(),
            mOrder,
            &gain);

    out->setSampleRate(sampleRate);

    out->setFBOrder(1);
    out->getFBData()[0] = pow(fabs(gain), 1.0f / 3.0f);
    
    out->setFFOrder(lpc.size());
    std::copy(lpc.begin(), lpc.end(), out->getFFData());

    // Filter frequency response.

    int nfft = mFFT.getInputLength();
    int outLength = mFFT.getOutputLength();

    mFFT.input(0) = 1.0f;

    for (int i = 0; i < lpc.size(); ++i)
        mFFT.input(i + 1) = lpc[i];

    for (int i = lpc.size() + 1; i < outLength; ++i)
        mFFT.input(i) = 0.0;

    mFFT.compute();

    outSpec->setSampleRate(sampleRate);
    outSpec->setLength(outLength);

    for (int i = 0; i < outLength; ++i) {
        outSpec->getData()[i] = fabs(out->getFBConstData()[0]) / std::abs(mFFT.output(i));
    }
}
