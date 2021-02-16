#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include "rpcxx.h"
#include <soxr.h>
#include <mutex>

namespace Module::Audio {

    class Resampler {
    public:
        static constexpr int chMono = 1;

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
        
    private:
        void createResampler();

        soxr_io_spec_t      mSoxrIoSpec;
        soxr_quality_spec_t mSoxrQualitySpec;
        soxr_runtime_spec_t mSoxrRuntimeSpec;
        soxr_t mSoxr;

        std::mutex mMutex;

        int mInRate, mOutRate;
    };

}

#endif // AUDIO_RESAMPLER_H
