#ifndef NODES_NODEIO_AUDIOSPEC_H
#define NODES_NODEIO_AUDIOSPEC_H

#include "nodeio.h"

namespace Nodes::IO {

    class AudioSpec : public NodeIO {
    public:
        AudioSpec();

        void setSampleRate(int sampleRate);
        void setLength(int length);
        double *getData();

        int getSampleRate() const;
        int getLength() const;
        const double *getConstData() const;

    private:
        int mSampleRate;
        int mLength;

        std::vector<double> mData;
    };

}

#endif // NODES_NODEIO_AUDIOTIME_H
