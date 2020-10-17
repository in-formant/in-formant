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
        double *getFFData();
        double *getFBData();

        int getSampleRate() const;
        int getFFOrder() const;
        int getFBOrder() const;
        const double *getFFConstData() const;
        const double *getFBConstData() const;

    private:
        int mSampleRate;
        int mFFOrder;
        int mFBOrder;

        double *mFF;
        double *mFB;
    };

}

#endif // NODES_NODEIO_IIRFILTER_H
