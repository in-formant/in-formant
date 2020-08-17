#include "resampler.h"

using namespace Nodes;

Resampler::Resampler(int initialInSampleRate, int outSampleRate)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        }),
      mResampler(initialInSampleRate, outSampleRate)
{
}

void Resampler::setOutputSampleRate(int outSampleRate)
{
    mResampler.setOutputRate(outSampleRate);
}

int Resampler::getRequiredInputLength(int outLength)
{
    return mResampler.getRequiredInLength(outLength);
}

void Resampler::process(const NodeIO *inputs, NodeIO *outputs)
{
    auto in = inputs[0].as<IO::AudioTime>();
    auto out = outputs[0].as<IO::AudioTime>();

    if (mResampler.getInputRate() != in->getSampleRate()) {
        mResampler.setInputRate(in->getSampleRate());
    }

    int inLength = in->getLength();
    int outLength = mResampler.getExpectedOutLength(inLength);

    out->setLength(outLength);
    out->setSampleRate(mResampler.getOutputRate());

    mResampler.clear();
    mResampler.process(in->getConstData(), inLength, out->getData(), outLength);
}

