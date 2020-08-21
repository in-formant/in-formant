#include "node.h"
#include <stdexcept>

using namespace Nodes;

std::unique_ptr<NodeIO> Nodes::makeNodeIO(NodeIOType type)
{
    switch (type) {
    case NODE_IO_TYPE_AUDIO_TIME:
        return std::make_unique<IO::AudioTime>();
    case NODE_IO_TYPE_AUDIO_SPEC:
        return std::make_unique<IO::AudioSpec>();
    case NODE_IO_TYPE_FREQUENCIES:
        return std::make_unique<IO::Frequencies>();
    default:
        throw std::runtime_error("Nodes] Unrecognized node I/O type");
    }
}

NodeIO::~NodeIO()
{
}
