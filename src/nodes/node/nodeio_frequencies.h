#ifndef NODES_NODEIO_FREQUENCIES_H
#define NODES_NODEIO_FREQUENCIES_H

#include "nodeio.h"
#include <vector>

namespace Nodes::IO {

    class Frequencies : public NodeIO {
    public:
        Frequencies();

        void setLength(int length);
        void set(int index, float frequency);

        int getLength() const;
        const float get(int index) const;

    private:
        int mLength;
        std::vector<float> mData;
    };

}

#endif // NODES_NODEIO_FREQUENCIES_H
