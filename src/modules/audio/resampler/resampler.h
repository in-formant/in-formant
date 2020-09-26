#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <soxr.h>
#include <vector>

namespace Module::Audio {

    class Resampler {
    public:
        static constexpr int chMono = 1;

        Resampler(int inRate, int outRate);
        ~Resampler();

        constexpr int getNumChannels() const { return chMono; }

        void setInputRate(int inRate);
        void setOutputRate(int outRate);
        void setRate(int inRate, int outRate);
        void setQuality(int quality);

        int getInputRate() const;
        int getOutputRate() const;
        void getRate(int *pInRate, int *pOutRate) const;
        int getQuality() const;

        int getRequiredInLength(int outLength);
        int getExpectedOutLength(int inLength);
    
        int getDelay() const;

        void clear();
        std::vector<float> process(const float *pIn, int inLength);
        
    private:
        void createResampler();

        soxr_io_spec_t mSoxrIoSpec;
        soxr_quality_spec_t mSoxrQualitySpec;
        soxr_runtime_spec_t mSoxrRuntimeSpec;
        soxr_t mSoxr;

        int mInRate, mOutRate;

        soxr_error_t err;
        void checkError();
    };

}

#endif // AUDIO_RESAMPLER_H
