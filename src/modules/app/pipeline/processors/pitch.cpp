#include "pitch.h"

#include "../../../../analysis/analysis.h"

using namespace Module::App::Processors;

Pitch::Pitch(Main::Config *config, Main::DataStore *dataStore,
            std::shared_ptr<Analysis::PitchSolver>& pitchSolver)
    : BaseProcessor(config->getAnalysisPitchSpacing(),
                    config->getAnalysisPitchWindow()),
      mConfig(config),
      mDataStore(dataStore),
      mPitchSolver(pitchSolver)
{
}

void Pitch::processData(const rpm::vector<double>& data, double sampleRate)
{
    auto pitchResult = mPitchSolver->solve(data.data(), (int) data.size(), sampleRate);

    mDataStore->beginWrite();

    if (pitchResult.voiced) {
        mDataStore->getPitchTrack().insert(getCenteredTime(), pitchResult.pitch);
    }
    else {
        mDataStore->getPitchTrack().insert(getCenteredTime(), std::nullopt);
    }

    mDataStore->endWrite();
}