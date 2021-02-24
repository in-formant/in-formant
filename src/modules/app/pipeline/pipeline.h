#ifndef APP_PIPELINE_H
#define APP_PIPELINE_H

#include "rpcxx.h"
#include "../../../analysis/analysis.h"
#include "../../../nodes/nodes.h"
#include "../../audio/audio.h"
#include "../../../context/datastore.h"
#include "../../../context/config.h"

#include <atomic>
#include <thread>
#include <chrono>

namespace Module::App
{
    using millis = std::chrono::milliseconds;
    using namespace std::chrono_literals;

    class Pipeline {
    public:
        Pipeline(Module::Audio::Buffer *captureBuffer,
                Main::DataStore *dataStore, Main::Config *config,
                std::shared_ptr<Analysis::PitchSolver> pitchSolver,
                std::shared_ptr<Analysis::LinpredSolver> linpredSolver,
                std::shared_ptr<Analysis::FormantSolver> formantSolver,
                std::shared_ptr<Analysis::InvglotSolver> invglotSolver);
        ~Pipeline();

        void processAll();

    private:
        Module::Audio::Buffer *captureBuffer;
        Main::DataStore *dataStore;
        Main::Config *config;

        std::shared_ptr<Analysis::PitchSolver> pitchSolver;
        std::shared_ptr<Analysis::LinpredSolver> linpredSolver;
        std::shared_ptr<Analysis::FormantSolver> formantSolver;
        std::shared_ptr<Analysis::InvglotSolver> invglotSolver;

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

