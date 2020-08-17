#include "nodeio.h"
#include "nodeio_audiotime.h"
#include "nodeio_audiospec.h"
#include <stdexcept>

using namespace Nodes;

std::unique_ptr<NodeIO> Nodes::makeNodeIO(NodeIOType type)
{
    switch (type) {
    case NODE_IO_TYPE_AUDIO_TIME:
        return std::make_unique<IO::AudioTime>();
    case NODE_IO_TYPE_AUDIO_SPEC:
        return std::make_unique<IO::AudioSpec>();
    default:
        throw std::runtime_error("Nodes] Unrecognized node I/O type");
    }
}

NodeIO::~NodeIO()
{
}
