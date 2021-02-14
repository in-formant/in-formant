#include "contextmanager.h"
#include "timings.h"

#include <iostream>

using namespace Main;
using namespace std::chrono_literals;

ContextManager::ContextManager(
                int captureSampleRate,
                const dur_ms &playbackBlockMinDuration,
                const dur_ms &playbackBlockMaxDuration,
                const dur_ms &playbackDuration,
                int playbackSampleRate
            )
    : mConfig(std::make_unique<Config>()),
      mPitchSolver(makePitchSolver(mConfig->getPitchAlgorithm())),
      mLinpredSolver(makeLinpredSolver(mConfig->getLinpredAlgorithm())),
      mFormantSolver(makeFormantSolver(mConfig->getFormantAlgorithm())),
      mInvglotSolver(makeInvglotSolver(mConfig->getInvglotAlgorithm())),
      mCaptureBuffer(std::make_unique<Audio::Buffer>(captureSampleRate)),
      mPlaybackQueue(std::make_unique<Audio::Queue>(
                  playbackBlockMinDuration.count(), playbackBlockMaxDuration.count(),
                  playbackDuration.count(), playbackSampleRate, [](auto...){})),
      mDataStore(std::make_unique<DataStore>()),
      mPipeline(std::make_unique<App::Pipeline>(
                  mCaptureBuffer.get(), mDataStore.get(),
                  mPitchSolver, mLinpredSolver,
                  mFormantSolver, mInvglotSolver)),
      mSynthesizer(std::make_unique<App::Synthesizer>(mPlaybackQueue.get())),
      mAudioContext(std::make_unique<AudioContext>(
                  mConfig->getAudioBackend(),
                  mCaptureBuffer.get(),
                  mPlaybackQueue.get())),
      mRenderContext(std::make_unique<RenderContext>(mConfig.get(), mDataStore.get())),
      mGuiContext(std::make_unique<GuiContext>(mConfig.get(), mRenderContext.get()))
{
    createViews();
    loadConfig();
    mDataStore->setFormantTrackCount(4);
    QObject::connect(mConfig.get(), &Config::pitchAlgorithmChanged,
            [this](int index) {
            std::cout << index << std::endl;
                mPitchSolver.reset(makePitchSolver(static_cast<PitchAlgorithm>(index)));
            });
    QObject::connect(mConfig.get(), &Config::linpredAlgorithmChanged,
            [this](int index) {
                mLinpredSolver.reset(makeLinpredSolver(static_cast<LinpredAlgorithm>(index)));
            });
    QObject::connect(mConfig.get(), &Config::formantAlgorithmChanged,
            [this](int index) {
                mFormantSolver.reset(makeFormantSolver(static_cast<FormantAlgorithm>(index)));
            });
    QObject::connect(mConfig.get(), &Config::invglotAlgorithmChanged,
            [this](int index) {
                mInvglotSolver.reset(makeInvglotSolver(static_cast<InvglotAlgorithm>(index)));
            });
    QObject::connect(mConfig.get(), &Config::audioBackendChanged,
            [this](int index) {
                mAudioContext = std::make_unique<AudioContext>(
                        static_cast<Module::Audio::Backend>(index),
                        mCaptureBuffer.get(),
                        mPlaybackQueue.get());
                openAndStartAudioStreams();
            });
}

int ContextManager::exec()
{
    openAndStartAudioStreams();
    startAnalysisThread();

    int retCode = mGuiContext->exec();

    stopAnalysisThread();

    return retCode;
}

void ContextManager::createViews()
{
    mViews["spectrogram"] = std::make_unique<View::Spectrogram>();

    setView("spectrogram");
}

void ContextManager::loadConfig()
{
    mViewMinFrequency = mConfig->getViewMinFrequency();
    mViewMaxFrequency = mConfig->getViewMaxFrequency();
    mViewFftSize = mConfig->getViewFFTSize();

    mAnalysisMaxFrequency = mConfig->getAnalysisMaxFrequency();
    mAnalysisLpOffset = mConfig->getAnalysisLpOffset();
    mAnalysisLpOrder = std::round((double) mAnalysisMaxFrequency / 500.0) + mAnalysisLpOffset;
    mAnalysisPitchSampleRate = mConfig->getAnalysisPitchSampleRate();
}

void ContextManager::openAndStartAudioStreams()
{
    mAudioContext->openCaptureStream(nullptr);
    mAudioContext->startCaptureStream();
    mAudioContext->openPlaybackStream(nullptr);
    mAudioContext->startPlaybackStream();
}

void ContextManager::startAnalysisThread()
{
    mAnalysisRunning = true;
    mAnalysisThread = std::thread(std::mem_fn(&ContextManager::analysisThreadLoop), this);
}

void ContextManager::analysisThreadLoop()
{
    while (mAnalysisRunning) {
        if (auto timer = timer_guard(timings::update)) {
            mAudioContext->tickAudio();
            mPipeline->processAll();
        }
    }
}

void ContextManager::stopAnalysisThread()
{
    if (mAnalysisThread.joinable()) {
        mAnalysisRunning = false;
        mAnalysisThread.join();
    }
}

void ContextManager::setView(const std::string &name)
{
    AbstractView *view;

    auto it = mViews.find(name);    
    if (it != mViews.end()) {
        view = it->second.get();
    }
    else {
        view = nullptr;
    }
    
    mGuiContext->setView(view);
    mRenderContext->setView(view);
}
