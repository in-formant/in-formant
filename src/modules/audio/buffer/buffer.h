#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include "../resampler/resampler.h"
#include <deque>
#include <mutex>

namespace Module::Audio {

    class Buffer {
    public:
        Buffer(int sampleRate, int durationInMs);

        void setSampleRate(int sampleRate);
        void setDuration(int durationInMs);
        void setLength(int length);

        int getSampleRate() const;
        int getDuration() const;
        int getLength() const;

        void pull(double *pOut, int outLength);
        void push(const float *pIn, int inLength);

    private:
        int mSampleRate;

        int mLength;
        std::deque<double> mData;

        std::mutex mLock;
    };

}

#endif // AUDIO_BUFFER_H
