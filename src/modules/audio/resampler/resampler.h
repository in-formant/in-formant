#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "rpcxx.h"
#include <soxr.h>
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
    
        int getDelay() const;

        void clear();
        rpm::vector<double> process(const double *pIn, int inLength);
       
        static rpm::vector<double> oneShot(const double *pIn, int inLength, int src, int dst);

    private:
        void createResampler();
        
        int mId;

        soxr_io_spec_t      mSoxrIoSpec;
        soxr_quality_spec_t mSoxrQualitySpec;
        soxr_runtime_spec_t mSoxrRuntimeSpec;
        soxr_t mSoxr;

        std::mutex mMutex;

        int mInRate, mOutRate;

        static std::atomic_int sId;
    };

}

#endif // AUDIO_RESAMPLER_H
