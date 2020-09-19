#include "preemph.h"
#include "../../modules/math/constants.h"
#include <iostream>

using namespace Nodes;

PreEmphasis::PreEmphasis(float frequency)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        }),
      mFrequency(frequency)
{
}

void PreEmphasis::setFrequency(float frequency)
{
    mFrequency = frequency;
}

void PreEmphasis::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioTime>();

    int sampleRate = in->getSampleRate();
    int length = in->getLength();

    out->setSampleRate(sampleRate);
    out->setLength(length);

    float factor = expf(-(2.0f * M_PI * mFrequency) / sampleRate);

    const float *x = in->getConstData();
    float *y = out->getData();

    for (int i = 0; i < length; ++i) {
        y[i] = x[i];
    }
    
    for (int i = length - 1; i >= 1; --i) {
        y[i] -= factor * y[i - 1];
    }
}
