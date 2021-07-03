#include "oscilloscope.h"

#include "../../../../analysis/analysis.h"

using namespace Module::App::Processors;

Oscilloscope::Oscilloscope(Main::Config *config, Main::DataStore *dataStore,
            std::shared_ptr<Analysis::InvglotSolver>& invglotSolver)
    : BaseProcessor(config->getAnalysisOscilloscopeSpacing(),
                    config->getAnalysisOscilloscopeWindow()),
      mConfig(config),
      mDataStore(dataStore),
      mInvglotSolver(invglotSolver)
{
}

void Oscilloscope::processData(const rpm::vector<double>& data, double sampleRate)
{
    constexpr double fsOsc = 8000;

    mResampler.setRate(sampleRate, fsOsc);

    auto out = mResampler.process(data);
    auto invglotResult = mInvglotSolver->solve(out.data(), (int) out.size(), fsOsc);

    mDataStore->beginWrite();
   
    mDataStore->getSoundTrack().insert(getCenteredTime(), out);
    mDataStore->getGifTrack().insert(getCenteredTime(), invglotResult.glotSig);

    mDataStore->endWrite();
}