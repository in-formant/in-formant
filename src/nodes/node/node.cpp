#include "node.h"
#include "nodeio_audiotime.h"
#include <cstring>

using namespace Nodes;

Node::Node(NodeDescriptor descriptor)
    : mInputTypes(descriptor.inputs, std::next(descriptor.inputs, descriptor.inputCount)),
      mOutputTypes(descriptor.outputs, std::next(descriptor.outputs, descriptor.outputCount))
{
}

Node::~Node()
{
}

const std::vector<NodeIOType> Node::getInputTypes() const
{
    return mInputTypes;
}

const std::vector<NodeIOType> Node::getOutputTypes() const
{
    return mOutputTypes;
}

NodePassthru::NodePassthru()
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioTime,
        })
{
}

void NodePassthru::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioTime>();

    out->setSampleRate(in->getSampleRate());
    out->setLength(in->getLength());

    memcpy(out->getData(), in->getConstData(), in->getLength() * sizeof(double));
}

