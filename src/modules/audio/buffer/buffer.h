#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include "rpcxx.h"
#include "../resampler/resampler.h"
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace Module::Audio {

    /*
     *  NOTE: There can be only one reader and one writer at a given time.
     */
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

        int mRequestedLength;

        std::condition_variable mCv;
        std::mutex mMutex;

        static std::atomic_bool sCancel;
    };

}

#endif // AUDIO_BUFFER_H
