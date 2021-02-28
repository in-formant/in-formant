#ifndef AUDIO_QUEUE_H
#define AUDIO_QUEUE_H

#include "rpcxx.h"
#include "../../../atomicops.h"
#include "../../../readerwriterqueue.h"
#include "../resampler/resampler.h"
#include <functional>
#include <mutex>
#include <atomic>

namespace Module::Audio {

    using QueueCallback = std::function<void(double*, int, void*)>;

    class Queue {
    public:
        Queue(int blockDurationInMs, int inSampleRate, QueueCallback callback);
        ~Queue();

        void setOutSampleRate(int outSampleRate);
        
        void setCallback(QueueCallback callback);

        int getInSampleRate() const;
        int getOutSampleRate() const;

        void pushIfNeeded(void *userdata);
        void pull(float *pOut, int outLength);

    private: 
        std::atomic<float> mBlockDuration;

        Resampler mResampler;
        QueueCallback mCallback;
        
        moodycamel::BlockingReaderWriterQueue<double> mQueue;
        
        std::mutex mLock;
    };

}

#endif // AUDIO_QUEUE_H
