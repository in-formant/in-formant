#ifndef NODES_NODEIO_AUDIOTIME_H
#define NODES_NODEIO_AUDIOTIME_H

#include "nodeio.h"

namespace Nodes::IO {

    class AudioTime : public NodeIO {
    public:
        AudioTime();
        
        void setSampleRate(int sampleRate);
        void setLength(int length);
        float *getData();

        int getSampleRate() const;
        int getLength() const;
        const float *getConstData() const;

    private:
        int mSampleRate;
        int mLength;

        std::vector<float> mData;
    };

}

#endif // NODES_NODEIO_AUDIOTIME_H
