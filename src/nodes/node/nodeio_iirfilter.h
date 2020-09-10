#ifndef NODES_NODEIO_IIRFILTER_H
#define NODES_NODEIO_IIRFILTER_H

#include "nodeio.h"

namespace Nodes::IO {

    class IIRFilter : public NodeIO {
    public:
        IIRFilter();
        ~IIRFilter();

        void setSampleRate(int sampleRate);
        void setFFOrder(int ffOrder);
        void setFBOrder(int fbOrder);
        float *getFFData();
        float *getFBData();

        int getSampleRate() const;
        int getFFOrder() const;
        int getFBOrder() const;
        const float *getFFConstData() const;
        const float *getFBConstData() const;

    private:
        int mSampleRate;
        int mFFOrder;
        int mFBOrder;

        float *mFF;
        float *mFB;
    };

}

#endif // NODES_NODEIO_IIRFILTER_H
