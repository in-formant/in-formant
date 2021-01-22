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

namespace Main {

    using namespace Module;

    using dur_ms = std::chrono::milliseconds;

    extern int argc;
    extern char **argv;

    class ContextManager {
    public:
        ContextManager(
                int captureSampleRate,
                const dur_ms &captureDuration,
                const dur_ms &playbackBlockMinDuration,
                const dur_ms &playbackBlockMaxDuration,
                const dur_ms &playbackDuration,
                int playbackSampleRate);

        int exec();

    private:
        void createViews();
        void loadConfig();
        void updatePipeline();
        
        void startAnalysisThread();
        void analysisThreadLoop();
        void stopAnalysisThread();

        void setView(const std::string &name);

        std::unique_ptr<Audio::Buffer> mCaptureBuffer;
        std::unique_ptr<Audio::Queue> mPlaybackQueue;

        std::unique_ptr<App::Pipeline> mPipeline;
        std::unique_ptr<App::Synthesizer> mSynthesizer;

        std::unique_ptr<Config> mConfig;

        std::unique_ptr<Analysis::PitchSolver> mPitchSolver;
        std::unique_ptr<Analysis::LinpredSolver> mLinpredSolver;
        std::unique_ptr<Analysis::FormantSolver> mFormantSolver;
        std::unique_ptr<Analysis::InvglotSolver> mInvglotSolver;
        
        std::unique_ptr<DataStore> mDataStore;

        std::unique_ptr<AudioContext> mAudioContext;
        std::unique_ptr<RenderContext> mRenderContext;
        std::unique_ptr<GuiContext> mGuiContext;

        std::map<std::string, std::unique_ptr<AbstractView>> mViews;

        std::thread mAnalysisThread;
        std::atomic_bool mAnalysisRunning;

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
