#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include "rpcxx.h"
#include "../resampler/resampler.h"
#include <atomic>
#include <mutex>

namespace Module::Audio {

    class Buffer {
    public:
        Buffer(int sampleRate);

        void setSampleRate(int sampleRate);

        int getSampleRate() const;
        int getLength() const;

        void pull(double *pOut, int outLength);
        void push(const float *pIn, int inLength);

        static void cancelPulls();

    private:
        int mSampleRate;

        std::atomic_int mLength;
        rpm::deque<double> mData;

        std::mutex mLock;

        static std::atomic_bool sCancel;
    };

}

#endif // AUDIO_BUFFER_H
