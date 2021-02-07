#ifndef NODES_NODEIO_FREQUENCIES_H
#define NODES_NODEIO_FREQUENCIES_H

#include "nodeio.h"

namespace Nodes::IO {

    class Frequencies : public NodeIO {
    public:
        Frequencies();

        void setLength(int length);
        void set(int index, double frequency);

        int getLength() const;
        double get(int index) const;

    private:
        int mLength;
        rpm::vector<double> mData;
    };

}

#endif // NODES_NODEIO_FREQUENCIES_H
