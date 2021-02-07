#ifndef APP_PIPELINE_H
#define APP_PIPELINE_H

#include "rpcxx.h"
#include "../../../analysis/analysis.h"
#include "../../../nodes/nodes.h"
#include "../../audio/audio.h"
#include "../../../context/datastore.h"

#include <atomic>
#include <thread>
#include <chrono>

namespace Module::App
{
    using millis = std::chrono::milliseconds;
    using namespace std::chrono_literals;

    class Pipeline {
    public:
        Pipeline(Module::Audio::Buffer *captureBuffer, Main::DataStore *dataStore);
        ~Pipeline();

        Pipeline& setPitchSolver(Analysis::PitchSolver *);
        Pipeline& setInvglotSolver(Analysis::InvglotSolver *);
        Pipeline& setLinpredSolver(Analysis::LinpredSolver *);
        Pipeline& setFormantSolver(Analysis::FormantSolver *);

        void processAll();

    private:
        Module::Audio::Buffer *captureBuffer;
        Main::DataStore *dataStore;

        Analysis::PitchSolver *pitchSolver;
        Analysis::InvglotSolver *invglotSolver;
        Analysis::LinpredSolver *linpredSolver;
        Analysis::FormantSolver *formantSolver;

        std::atomic_int64_t time;
        bool runningThreads;
        
        Module::Audio::Buffer bufferSpectrogram;
        std::thread threadSpectrogram;
        void callbackSpectrogram();

        Module::Audio::Buffer bufferPitch;
        std::thread threadPitch;
        void callbackPitch();

        Module::Audio::Buffer bufferFormants;
        std::thread threadFormants;
        void callbackFormants();

        double fftSampleRate;
        int fftSize;

        double preEmphasisFrequency;
        
        double secondSampleRate;
        int lpSpecLpOrder;

        double formantSampleRate;
        int formantLpOrder;
    };
}

#endif // APP_PIPELINE_H

