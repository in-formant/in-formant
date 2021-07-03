#ifndef PIPELINE_PROCESSOR_BASE_H
#define PIPELINE_PROCESSOR_BASE_H

#include "rpcxx.h"

namespace Module::App::Processors {

    class BaseProcessor {
    public:
        BaseProcessor(double frameSpace, double frameLength);

        bool canProcess(double timeNow) const;

        void process(const rpm::vector<double>& slidingWindow, double sampleRate, double timeNow);

        virtual void processData(const rpm::vector<double>& data, double sampleRate) = 0;

        double getFrameSpace() const;
        double getFrameLength() const;

    protected:
        double getCenteredTime() const;

    private:
        double mFrameSpace;
        double mFrameLength;

        double mTime;

        rpm::vector<double> mData;
    };
}

#endif // PIPELINE_PROCESSOR_BASE_H