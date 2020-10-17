#include "prereqs.h"
#include "../../analysis/filter/filter.h"
#include <algorithm>
#include <iostream>

using namespace Nodes;
using Module::Audio::Buffer;

Prereqs::Prereqs(Buffer *buffer, int outDurationInMs, int minOutLength)
    : Node(NodeDescriptor {
            .inputCount = 0,
            .inputs = nullptr,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime
        }),
      mBuffer(buffer),
      mOutDuration(outDurationInMs),
      mMinOutLength(minOutLength)
{
    if (buffer->getDuration() < outDurationInMs) {
        throw std::runtime_error("Nodes::Prereqs] Input buffer duration is shorter than the requested output duration");
    }
}

void Prereqs::setOutputDuration(int outDuration)
{
    mOutDuration = outDuration;
}

void Prereqs::setMinimumOutputLength(int minOutLength)
{
    mMinOutLength = minOutLength;
}

void Prereqs::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto out = outputs[0]->as<IO::AudioTime>();

    int sampleRate = mBuffer->getSampleRate();
    int bufferLength = mBuffer->getLength();

    int actualLength = std::max({ bufferLength, mMinOutLength, (sampleRate * mOutDuration) / 1000 });

    if (mBuffer->getLength() < actualLength) {
        mBuffer->setLength(actualLength);
    }

    static std::vector<std::array<double, 6>> hpsos;
    static int lastSampleRate = 0;
    static int lastLength = 0;

    if (lastLength != actualLength || lastSampleRate != sampleRate) {
        lastLength = actualLength;
        lastSampleRate = sampleRate;
        hpsos = Analysis::butterworthHighpass(4, 80.0f, sampleRate);
    }

    out->setSampleRate(sampleRate);
    out->setLength(actualLength);

    std::vector<double> input(actualLength);
    mBuffer->pull(input.data(), actualLength);
    input = Analysis::sosfilter(hpsos, input);
    for (int i = 0; i < actualLength; ++i) {
        out->getData()[i] = input[i];
    }
}
