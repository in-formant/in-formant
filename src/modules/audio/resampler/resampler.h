#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <vector>
#include <memory>
#include <CDSPResampler.h>

namespace Module::Audio {

    class Resampler {
    public:
        static constexpr int chMono = 1;

        Resampler(int inRate, int outRate);

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
        std::vector<double> process(double *pIn, int inLength);
        
    private:
        void createResampler();

        std::shared_ptr<r8b::CDSPResampler> mResampler;

        int mInRate, mOutRate;
    };

}

#endif // AUDIO_RESAMPLER_H
