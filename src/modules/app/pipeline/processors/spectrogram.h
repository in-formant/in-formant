#ifndef PIPELINE_PROCESSOR_SPECTROGRAM_H
#define PIPELINE_PROCESSOR_SPECTROGRAM_H

#include "rpcxx.h"

#include <memory>

#include "../../../audio/resampler/resampler.h"
#include "../../../../analysis/fft/fft.h"
#include "../../../../context/config.h"
#include "../../../../context/datastore.h"

#include "base.h"

namespace Module::App::Processors {

    class Spectrogram : public BaseProcessor {
    public:
        Spectrogram(Main::Config *config, Main::DataStore *dataStore);
        
        void processData(const rpm::vector<double>& data, double sampleRate) override;

    private:
        Main::Config *mConfig;
        Main::DataStore *mDataStore;

        Module::Audio::Resampler mResampler;
        rpm::vector<double> mData;

        std::shared_ptr<Analysis::RealFFT> mFFT;
        rpm::vector<std::array<double, 6>> mHighpass;
        rpm::vector<rpm::vector<double>> mHighpassMemory;
        double mHighpassSampleRate;
        double mHold;
    };

}

#endif // PIPELINE_PROCESSOR_SPECTROGRAM_H