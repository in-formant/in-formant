#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "rpcxx.h"
#include <CDSPResampler.h>
#include <mutex>
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
        void setQuality(int quality);

        int getInputRate() const;
        int getOutputRate() const;
        void getRate(int *pInRate, int *pOutRate) const;
        int getQuality() const;

        int getRequiredInLength(int outLength) const;
        int getExpectedOutLength(int inLength) const;
    
        double getDelay() const;

        void clear();
        rpm::vector<double> process(const double *pIn, int inLength);

    private:
        void updateRatio();
        void setupResampler();
        
        int mId;

        std::mutex mMutex;
        std::unique_ptr<r8b::CDSPResampler> mResampler;
        int mInRate, mOutRate;

        static std::atomic_int sId;

        static int getInLenBeforeOutStart(int src, int dst, r8b::CDSPResampler& resampler);
        static rpm::map<std::pair<int, int>, int> sInLenBeforeOutStart;
    };

}

#endif // AUDIO_RESAMPLER_H
