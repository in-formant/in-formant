#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#define FLOATING_POINT
#ifdef __SSE__
#   define USE_SSE
#endif
#ifdef __ARM_NEON
#   define USE_NEON
#endif

#define RANDOM_PREFIX sa_speex
#define OUTSIDE_SPEEX
#include "speex_resampler/speex_resampler.h"

#ifdef __cplusplus
#   include <cstdint>
extern "C" {
#else
#   include <stdint.h>
#endif 
int speex_resampler_get_required_input_frame_count(SpeexResamplerState* st, uint64_t out_len, uint64_t* in_len);
int speex_resampler_get_expected_output_frame_count(SpeexResamplerState* st, uint64_t in_len, uint64_t* out_len);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace Module::Audio {

    class Resampler {
    public:
        static constexpr int chMono = 1;

        Resampler(int inRate, int outRate, int quality);
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

        void process(const float *pIn, uint32_t inLength, float *pOut, uint32_t outLength);
        
    private:
        SpeexResamplerState *mState;
        int mInRate, mOutRate;
        int mQuality;

        int err;
        void checkError();
    };

}

#endif // __cplusplus

#endif // AUDIO_RESAMPLER_H
