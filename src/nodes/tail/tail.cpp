#include "tail.h"
#include <cstring>

using namespace Nodes;

Tail::Tail(int outDurationInMs)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        }),
      mOutDuration(outDurationInMs)
{
}

void Tail::setOutputDuration(int outDuration)
{
    mOutDuration = outDuration;
}

void Tail::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioTime>();

    int sampleRate = in->getSampleRate();

    int inLength = in->getLength();
    int outLength = (sampleRate * mOutDuration) / 1000;

    out->setSampleRate(sampleRate);
    out->setLength(outLength);

    for (int i = 0; i < outLength; ++i) {
        out->getData()[i] = in->getConstData()[inLength - outLength + i];
    }
}
