#ifndef PIPELINE_PROCESSOR_FORMANTS_H
#define PIPELINE_PROCESSOR_FORMANTS_H

#include "rpcxx.h"

#include <memory>

#include "../../../audio/resampler/resampler.h"
#include "../../../../analysis/formant/formant.h"
#include "../../../../context/config.h"
#include "../../../../context/datastore.h"

#include "base.h"

namespace Module::App::Processors {

    class Formants : public BaseProcessor {
    public:
        Formants(Main::Config *config, Main::DataStore *dataStore,
            std::shared_ptr<Analysis::LinpredSolver>& linpredSolver,
            std::shared_ptr<Analysis::FormantSolver>& formantSolver);
        
        void processData(const rpm::vector<double>& data, double sampleRate) override;

    private:
        Main::Config *mConfig;
        Main::DataStore *mDataStore;
        std::shared_ptr<Analysis::LinpredSolver>& mLinpredSolver;
        std::shared_ptr<Analysis::FormantSolver>& mFormantSolver;

        rpm::vector<double> mWindow;
        Module::Audio::Resampler mResamplerLPC;
#ifdef ENABLE_TORCH
        Module::Audio::Resampler mResampler16k;
#endif

        double mLastSample;
    };

}

#endif // PIPELINE_PROCESSOR_FORMANTS_H