#ifndef AUDIO_QUEUE_H
#define AUDIO_QUEUE_H

#include "rpcxx.h"
#include "../resampler/resampler.h"
#include <functional>
#include <mutex>

namespace Module::Audio {

    using QueueCallback = std::function<void(double*, int, void*)>;

    class Queue {
    public:
        Queue(int minInDurationInMs, int maxInDurationInMs, int avgDurationInMs,
                int inSampleRate, QueueCallback callback);
        ~Queue();

        void setOutSampleRate(int outSampleRate);
        
        void setCallback(QueueCallback callback);

        int getInSampleRate() const;
        int getOutSampleRate() const;

        void pushIfNeeded(void *userdata);
        void pull(float *pOut, int outLength);

    private: 
        const int mMinInLength;
        const int mMaxInLength;
        const int mAvgLength;

        Resampler mResampler;
        QueueCallback mCallback;
        
        rpm::deque<double> mDeque;
        
        std::mutex mLock;
    };

}

#endif // AUDIO_QUEUE_H
