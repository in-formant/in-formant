#include "preemph.h"
#include <iostream>
#include <utility>
#include <cmath>

using namespace Nodes;

PreEmphasis::PreEmphasis()
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        })
{
}

void PreEmphasis::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioTime>();

    int sampleRate = in->getSampleRate();
    int length = in->getLength();

    out->setSampleRate(sampleRate);
    out->setLength(length);

    rpm::vector<double> x(in->getConstData(), in->getConstData() + length);
    
    rpm::vector<double> a;
    double g;

    /*do {
        a = lpc.solve(x.data(), length, 1, &g);
        if (g <= 0 || fabs(a[0]) < 0.001)
            break;

        for (int i = length - 1; i >= 1; --i) {
            x[i] += a[0] * x[i - 1];
        }
    } while (fabs(a[0]) >= 0.001);*/
    
    double factor = exp(-(2.0 * M_PI * 50.0) / sampleRate);
    for (int i = length - 1; i >= 1; --i) {
        x[i] -= factor * x[i - 1];
    }

    for (int i = 0; i < length; ++i) {
        out->getData()[i] = x[i];
    }
}
