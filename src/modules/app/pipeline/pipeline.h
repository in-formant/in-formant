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

    static const millis analysisThreadsWaitInterval = 100ms;
    static const millis analysisThreadsWaitDuration = 100ms;

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
        std::atomic_bool mThreadRunning;
        std::atomic_bool mStopThread;

        std::thread mProcessingThread;

        Module::Audio::Buffer mBuffer;
        double mSampleRate;

        double mSpectrogramTime;
        rpm::vector<double> mSpectrogramOverlap;
        rpm::vector<double> mSpectrogramData;
        Module::Audio::Resampler mSpectrogramResampler;
        std::unique_ptr<Analysis::RealFFT> mSpectrogramFFT;
        rpm::vector<std::array<double, 6>> mSpectrogramHighpass;
        rpm::vector<rpm::vector<double>> mSpectrogramHighpassMemory;
        double mSpectrogramHighpassSampleRate;
        double mSpectrogramHold; 

        double mPitchTime;
        rpm::vector<double> mPitchData;
        Module::Audio::Resampler mPitchResampler;
 
        double mFormantTime;
        rpm::vector<double> mFormantData;
        rpm::vector<double> mFormantWindow;
        Module::Audio::Resampler mFormantResamplerLPC;
        Module::Audio::Resampler mFormantResampler16k;
        
        double mOscilloscopeTime;
        rpm::vector<double> mOscilloscopeData;
        Module::Audio::Resampler mOscilloscopeResampler;

        void callbackProcessing();
        void processSpectrogram();
        void processPitch();
        void processFormants();
        void processOscilloscope();
    };
}

#endif // APP_PIPELINE_H

