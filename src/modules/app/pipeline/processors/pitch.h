#ifndef PIPELINE_PROCESSOR_PITCH_H
#define PIPELINE_PROCESSOR_PITCH_H

#include "rpcxx.h"

#include <memory>

#include "../../../../context/config.h"
#include "../../../../context/datastore.h"

#include "base.h"

namespace Module::App::Processors {

    class Pitch : public BaseProcessor {
    public:
        Pitch(Main::Config *config, Main::DataStore *dataStore,
            std::shared_ptr<Analysis::PitchSolver>& pitchSolver);
        
        void processData(const rpm::vector<double>& data, double sampleRate) override;

    private:
        Main::Config *mConfig;
        Main::DataStore *mDataStore;
        std::shared_ptr<Analysis::PitchSolver>& mPitchSolver;
    };

}

#endif // PIPELINE_PROCESSOR_PITCH_H