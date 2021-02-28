#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include "rpcxx.h"
#include "../../../atomicops.h"
#include "../../../readerwriterqueue.h"
#include "../resampler/resampler.h"
#include <atomic>

namespace Module::Audio {

    /*
     *  NOTE: There can be only one reader and one writer at a given time.
     */
    class Buffer {
    public:
        Buffer(double sampleRate = 0);

        void setSampleRate(double sampleRate);

        double getSampleRate() const;
        int getLength() const;

        void pull(double *pOut, int outLength);
        void push(const float *pIn, int inLength);

        static void cancelPulls();

    private:
        int mId;
        double mSampleRate;

        moodycamel::BlockingReaderWriterQueue<double> mQueue;

        static std::atomic_bool sCancel;
        static std::atomic_int sId;
    };

}

#endif // AUDIO_BUFFER_H
