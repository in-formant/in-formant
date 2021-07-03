#ifndef PIPELINE_PROCESSOR_OSCILLOSCOPE_H
#define PIPELINE_PROCESSOR_OSCILLOSCOPE_H

#include "rpcxx.h"

#include <memory>

#include "../../../audio/resampler/resampler.h"
#include "../../../../analysis/invglot/invglot.h"
#include "../../../../context/config.h"
#include "../../../../context/datastore.h"

#include "base.h"

namespace Module::App::Processors {

    class Oscilloscope : public BaseProcessor {
    public:
        Oscilloscope(Main::Config *config, Main::DataStore *dataStore,
            std::shared_ptr<Analysis::InvglotSolver>& invglotSolver);
        
        void processData(const rpm::vector<double>& data, double sampleRate) override;

    private:
        Main::Config *mConfig;
        Main::DataStore *mDataStore;
        std::shared_ptr<Analysis::InvglotSolver>& mInvglotSolver;

        Module::Audio::Resampler mResampler;
    };

}

#endif // PIPELINE_PROCESSOR_OSCILLOSCOPE_H