#include "preemph.h"
#include "../../modules/math/constants.h"

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

    float factor = 1.0f / (2.0f * M_PI * mFrequency / sampleRate + 1.0f);

    const float *x = in->getConstData();
    float *y = out->getData();

    y[0] = x[0];
    for (int i = 1; i < length; ++i) {
        y[i] = factor * (y[i - 1] + x[i] - x[i - 1]);
    }
}
