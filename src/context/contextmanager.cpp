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
#ifndef WITHOUT_SYNTH
      mSynthesizer(std::make_unique<App::Synthesizer>(mPlaybackQueue.get())),
#endif
      mAudioContext(std::make_unique<AudioContext>(
                  mConfig->getAudioBackend(),
                  mCaptureBuffer.get(),
                  mPlaybackQueue.get())),
      mRenderContext(std::make_unique<RenderContext>(mConfig.get(), mDataStore.get())),
#ifndef WITHOUT_SYNTH
      mGuiContext(std::make_unique<GuiContext>(mConfig.get(), mRenderContext.get(), &mSynthWrapper))
#else
      mGuiContext(std::make_unique<GuiContext>(mConfig.get(), mRenderContext.get()))
#endif
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
#ifndef WITHOUT_SYNTH
    startSynthesisThread();
#endif

    int retCode = mGuiContext->exec();

    stopAnalysisThread();
#ifndef WITHOUT_SYNTH
    stopSynthesisThread();
#endif

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
#ifndef WITHOUT_SYNTH
    mAudioContext->openPlaybackStream(nullptr);
    mAudioContext->startPlaybackStream();
#endif
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

#ifndef WITHOUT_SYNTH

void ContextManager::startSynthesisThread()
{
    mSynthesisRunning = true;
    mSynthesisThread = std::thread(std::mem_fn(&ContextManager::synthesisThreadLoop), this);
    mSynthPushThread = std::thread(std::mem_fn(&ContextManager::synthPushThreadLoop), this);
}

void ContextManager::synthPushThreadLoop()
{ 
    while (mSynthesisRunning) {
        mPlaybackQueue->pushIfNeeded(mSynthesizer.get());
        std::this_thread::sleep_for(20ms);
    }
}

void ContextManager::synthesisThreadLoop()
{
    using Synth = Module::App::Synthesizer;

    QObject::connect(&mSynthWrapper, &SynthWrapper::noiseGainChanged, mSynthesizer.get(), &Synth::setNoiseGain);
    QObject::connect(&mSynthWrapper, &SynthWrapper::glotGainChanged, mSynthesizer.get(), &Synth::setGlotGain);
    QObject::connect(&mSynthWrapper, &SynthWrapper::glotPitchChanged, mSynthesizer.get(), &Synth::setGlotPitch);
    QObject::connect(&mSynthWrapper, &SynthWrapper::glotRdChanged, mSynthesizer.get(), &Synth::setGlotRd);
    QObject::connect(&mSynthWrapper, &SynthWrapper::glotTcChanged, mSynthesizer.get(), &Synth::setGlotTc);
    QObject::connect(&mSynthWrapper, &SynthWrapper::filterShiftChanged, mSynthesizer.get(), &Synth::setFilterShift);
    QObject::connect(&mSynthWrapper, &SynthWrapper::voicedChanged, mSynthesizer.get(), &Synth::setVoiced);

    double maxFrequency = 0;
    rpm::vector<double> frequencies(1024);
    rpm::vector<std::complex<double>> wn(frequencies.size());

    double maxFrequencySource = 16000;
    Analysis::RealFFT fft(512);
    rpm::vector<double> fftFrequencies(fft.getOutputLength());
    for (int k = 0; k < fftFrequencies.size(); ++k) {
        fftFrequencies[k] = maxFrequencySource * (double) (k + 1) / (double) fftFrequencies.size();
    }
    
    while (mSynthesisRunning) {
        mDataStore->beginRead();

        if (mSynthWrapper.followPitch()) {
            if (!mDataStore->getPitchTrack().empty()) {
                std::optional<double> p = mDataStore->getPitchTrack().back();
                if (p.has_value()) {
                    mSynthWrapper.setVoiced(true);
                    mSynthWrapper.setGlotPitch(*p);
                }
                else {
                    mSynthWrapper.setVoiced(false);
                }
            }
            else {
                mSynthWrapper.setVoiced(false);
            }
        }

        if (mSynthWrapper.followFormants()) {
            rpm::vector<Analysis::FormantData> formants;
            for (int i = 0; i < 4; ++i) {
                if (!mDataStore->getFormantTrack(i).empty()) {
                    std::optional<double> fi = mDataStore->getFormantTrack(i).back();
                    if (fi.has_value()) {
                        formants.push_back({*fi, 100.0});
                    }
                }
            }
            mSynthesizer->setFormants(formants);
        }

        mDataStore->endRead();

        if (mSynthWrapper.enabled()) {
            mSynthesizer->setMasterGain(1.0);
        }
        else {
            mSynthesizer->setMasterGain(0.0);
        }

        double synthSampleRate;
        auto filter = mSynthesizer->getFilterCopy(&synthSampleRate);

        if (maxFrequency != synthSampleRate / 2) {
            maxFrequency = synthSampleRate / 2;
            for (int k = 0; k < frequencies.size(); ++k) {
                frequencies[k] = maxFrequency * (double) (k + 1) / (double) frequencies.size();
                wn[k] = std::polar(1.0, 2.0 * M_PI * frequencies[k] / synthSampleRate);
            }
        }

        mSynthWrapper.setFilterResponse(frequencies, Analysis::sosfreqz(filter, wn));
        
        auto source = mSynthesizer->getSourceCopy(maxFrequencySource * 2, 25.0);
        mSynthWrapper.setSource(source, maxFrequencySource * 2);

        auto sourceSpectrum = Analysis::fft_n(fft, source);
        mSynthWrapper.setSourceSpectrum(fftFrequencies, sourceSpectrum);

        std::this_thread::sleep_for(50ms);
    }
}

void ContextManager::stopSynthesisThread()
{
    if (mSynthesisThread.joinable() || mSynthPushThread.joinable()) {
        mSynthesisRunning = false;
        if (mSynthesisThread.joinable()) {
            mSynthesisThread.join();
        }
        if (mSynthPushThread.joinable()) {
            mSynthPushThread.join();
        }
    }
}

#endif // !WITHOUT_SYNTH

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
