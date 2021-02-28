#ifndef MAIN_CONTEXT_MANAGER_H
#define MAIN_CONTEXT_MANAGER_H

#include "../analysis/analysis.h"
#include "../modules/app/app.h"
#include "audiocontext.h"
#include "rendercontext.h"
#include "guicontext.h"
#include "datastore.h"
#include "views/views.h"
#include "config.h"
#include <atomic>
#include <thread>

#include "synthwrapper.h"

#define STR(arg) #arg
#define INFORMANT_VERSION_STR STR(INFORMANT_VERSION)

namespace Main {

    using namespace Module;

    using dur_ms = std::chrono::milliseconds;

    extern int argc;
    extern char **argv;

    class ContextManager {
    public:
        ContextManager(
                int captureSampleRate,
                const dur_ms &playbackBlockDuration,
                int playbackSampleRate);

        int exec();

    private:
        void createViews();
        void loadConfig();
        
        void openAndStartAudioStreams();
        
        void startAnalysisThread();
        void analysisThreadLoop();
        void stopAnalysisThread();

#ifndef WITHOUT_SYNTH
        void startSynthesisThread();
        void synthesisThreadLoop();
        void synthPushThreadLoop();
        void stopSynthesisThread();
#endif

        void setView(const std::string &name);

        std::unique_ptr<Config> mConfig;

        std::shared_ptr<Analysis::PitchSolver> mPitchSolver;
        std::shared_ptr<Analysis::LinpredSolver> mLinpredSolver;
        std::shared_ptr<Analysis::FormantSolver> mFormantSolver;
        std::shared_ptr<Analysis::InvglotSolver> mInvglotSolver;
        
        std::unique_ptr<Audio::Buffer> mCaptureBuffer;
        std::unique_ptr<Audio::Queue> mPlaybackQueue;

        std::unique_ptr<DataStore> mDataStore;
        
        std::unique_ptr<App::Pipeline> mPipeline;
       
#ifndef WITHOUT_SYNTH
        std::unique_ptr<App::Synthesizer> mSynthesizer;
        SynthWrapper mSynthWrapper;
#endif

        std::unique_ptr<AudioContext> mAudioContext;
        std::unique_ptr<RenderContext> mRenderContext;
        std::unique_ptr<GuiContext> mGuiContext;

        rpm::map<std::string, std::unique_ptr<AbstractView>> mViews;

        std::thread mAnalysisThread;
        std::atomic_bool mAnalysisRunning;

#ifndef WITHOUT_SYNTH
        std::thread mSynthesisThread;
        std::thread mSynthPushThread;
        std::atomic_bool mSynthesisRunning;
#endif

        int mViewMinFrequency;
        int mViewMaxFrequency;
        int mViewFftSize;

        int mAnalysisMaxFrequency;
        int mAnalysisLpOffset;
        int mAnalysisLpOrder;
        int mAnalysisPitchSampleRate;
    };

}

#endif // MAIN_CONTEXT_MANAGER_H
