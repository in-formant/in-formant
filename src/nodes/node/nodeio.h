#ifndef NODES_NODEIO_H
#define NODES_NODEIO_H

#include <memory>

namespace Nodes {

    enum NodeIOType {
        // Audio data in the time domain.
        NODE_IO_TYPE_AUDIO_TIME,

        // Audio data in the frequency domain.
        NODE_IO_TYPE_AUDIO_SPEC,
    };

    static const NodeIOType kNodeIoTypeAudioTime = NODE_IO_TYPE_AUDIO_TIME;
    static const NodeIOType kNodeIoTypeAudioSpec = NODE_IO_TYPE_AUDIO_SPEC;

    namespace IO {
        struct AudioTimeDescriptor {
            int sampleRate;
            int length;
        };

        struct AudioSpecDescriptor {
            int sampleRate;
            int length;
        };
    }

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

};

#endif // NODES_NODEIO_H