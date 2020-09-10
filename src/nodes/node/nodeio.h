#ifndef NODES_NODEIO_H
#define NODES_NODEIO_H

#include <memory>
#include <vector>

namespace Nodes {

    enum NodeIOType : unsigned int {
        // Audio data in the time domain.
        NODE_IO_TYPE_AUDIO_TIME,

        // Audio data in the frequency domain.
        NODE_IO_TYPE_AUDIO_SPEC,

        // List of frequencies. (pitch; formants)
        NODE_IO_TYPE_FREQUENCIES,

        // IIR filter.
        NODE_IO_TYPE_IIR_FILTER,
    };

    static constexpr NodeIOType kNodeIoTypeAudioTime   = NODE_IO_TYPE_AUDIO_TIME;
    static constexpr NodeIOType kNodeIoTypeAudioSpec   = NODE_IO_TYPE_AUDIO_SPEC;
    static constexpr NodeIOType kNodeIoTypeFrequencies = NODE_IO_TYPE_FREQUENCIES;
    static constexpr NodeIOType kNodeIoTypeIIRFilter   = NODE_IO_TYPE_IIR_FILTER;

    class NodeIO {
    public:
        virtual ~NodeIO() = 0;

        template<typename T>
        inline T *as() {
            return static_cast<T *>(this); 
        }

        template<typename T>
        inline const T *as() const {
            return static_cast<const T *>(this);
        }
    };

    std::unique_ptr<NodeIO> makeNodeIO(NodeIOType type);
    std::vector<std::unique_ptr<NodeIO>> makeNodeIO(int count, ...);

    NodeIO **unpack(std::vector<std::unique_ptr<NodeIO>>& v, NodeIO ***ios);

};

#endif // NODES_NODEIO_H
