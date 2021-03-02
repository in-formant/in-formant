#ifndef APP_PIPELINE_H
#define APP_PIPELINE_H

#include "rpcxx.h"
#include "../../../analysis/analysis.h"
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
                std::shared_ptr<Analysis::PitchSolver>& pitchSolver,
                std::shared_ptr<Analysis::LinpredSolver>& linpredSolver,
                std::shared_ptr<Analysis::FormantSolver>& formantSolver,
                std::shared_ptr<Analysis::InvglotSolver>& invglotSolver);
        ~Pipeline();

        void processAll();

    private:
        Module::Audio::Buffer *mCaptureBuffer;
        Main::DataStore *mDataStore;
        Main::Config *mConfig;

        std::shared_ptr<Analysis::PitchSolver>& mPitchSolver;
        std::shared_ptr<Analysis::LinpredSolver>& mLinpredSolver;
        std::shared_ptr<Analysis::FormantSolver>& mFormantSolver;
        std::shared_ptr<Analysis::InvglotSolver>& mInvglotSolver;

        std::atomic<double> mTime;
        std::atomic_bool mRunningThreads;
        std::atomic_bool mStopThreads;
        
        Module::Audio::Buffer mBufferSpectrogram;
        std::thread mThreadSpectrogram;
        void callbackSpectrogram();
        Module::Audio::Resampler mSpectrumResampler;
        std::unique_ptr<Analysis::RealFFT> mSpectrumFFT;

        Module::Audio::Buffer mBufferPitch;
        std::thread mThreadPitch;
        void callbackPitch();

        Module::Audio::Buffer mBufferFormants;
        std::thread mThreadFormants;
        void callbackFormants();
    };
}

#endif // APP_PIPELINE_H

