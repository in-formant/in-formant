#ifndef NODES_NODEIO_AUDIOTIME_H
#define NODES_NODEIO_AUDIOTIME_H

#include "nodeio.h"

namespace Nodes::IO {

    class AudioTime : public NodeIO {
    public:
        AudioTime();
        
        void setSampleRate(int sampleRate);
        void setLength(int length);
        double *getData();

        int getSampleRate() const;
        int getLength() const;
        const double *getConstData() const;

    private:
        int mSampleRate;
        int mLength;

        rpm::vector<double> mData;
    };

}

#endif // NODES_NODEIO_AUDIOTIME_H
