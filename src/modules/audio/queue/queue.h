#ifndef AUDIO_QUEUE_H
#define AUDIO_QUEUE_H

#include "../resampler/resampler.h"
#include <functional>
#include <deque>
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
        
        std::deque<double> mDeque;
        
        std::mutex mLock;
    };

}

#endif // AUDIO_QUEUE_H
