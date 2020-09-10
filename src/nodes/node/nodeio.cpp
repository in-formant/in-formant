#include "node.h"
#include <stdexcept>
#include <cstdarg>

using namespace Nodes;

NodeIO::~NodeIO()
{
}

std::unique_ptr<NodeIO> Nodes::makeNodeIO(NodeIOType type)
{
    switch (type) {
    case NODE_IO_TYPE_AUDIO_TIME:
        return std::make_unique<IO::AudioTime>();
    case NODE_IO_TYPE_AUDIO_SPEC:
        return std::make_unique<IO::AudioSpec>();
    case NODE_IO_TYPE_FREQUENCIES:
        return std::make_unique<IO::Frequencies>();
    case NODE_IO_TYPE_IIR_FILTER:
        return std::make_unique<IO::IIRFilter>();
    default:
        throw std::runtime_error("Nodes] Unrecognized node I/O type");
    }
}

std::vector<std::unique_ptr<NodeIO>> Nodes::makeNodeIO(int count, ...)
{
    std::va_list args;
    va_start(args, count);

    std::vector<std::unique_ptr<NodeIO>> nodes;

    for (int i = 0; i < count; ++i) {
        NodeIOType type = static_cast<NodeIOType>(va_arg(args, unsigned int));
        nodes.push_back(makeNodeIO(type));
    }

    va_end(args);

    return nodes;
}

NodeIO **Nodes::unpack(std::vector<std::unique_ptr<NodeIO>>& v, NodeIO *** ios)
{
    if (*ios != nullptr) {
        delete[] *ios;
    }

    *ios = new NodeIO *[v.size()];

    for (int i = 0; i < v.size(); ++i) {
        (*ios)[i] = v[i].get();
    }

    return *ios;
}
