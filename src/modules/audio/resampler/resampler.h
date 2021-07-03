#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "rpcxx.h"
#include <samplerate.h>
#include <atomic>

namespace Module::Audio {

    class Resampler {
    public:
        static constexpr int chMono = 1;

        Resampler(int inRate = 0);
        Resampler(int inRate, int outRate);
        virtual ~Resampler();

        constexpr int getNumChannels() const { return chMono; }

        void setInputRate(int inRate);
        void setOutputRate(int outRate);
        void setRate(int inRate, int outRate);

        int getInputRate() const;
        int getOutputRate() const;
        void getRate(int *pInRate, int *pOutRate) const;

        int getRequiredInLength(int outLength) const;
        int getExpectedOutLength(int inLength) const;

        rpm::vector<double> process(const rpm::vector<double>& in);

    private:
        void updateRatio();
        void setupResampler();
        
        int mId;

        SRC_STATE *mSrc;
        int mInRate, mOutRate;

        static std::atomic_int sId;
    };

}

#endif // AUDIO_RESAMPLER_H
