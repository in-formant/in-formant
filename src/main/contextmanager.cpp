#include "contextmanager.h"
#include <thread>

using namespace Main;
using namespace std::chrono;
using namespace std::chrono_literals;
using Clock = steady_clock;

ContextManager::ContextManager(std::unique_ptr<Context>&& ctx)
    : ctx(std::move(ctx)),
      ndi(nullptr), ndo(nullptr)
{
}

ContextManager::~ContextManager()
{
    if (ndi != nullptr) delete[] ndi;
    if (ndo != nullptr) delete[] ndo;
}

void ContextManager::initialize()
{
    ctx->audio->initialize();
    ctx->audio->refreshDevices();
    
    ctx->target->initialize();
    ctx->target->setTitle("Speech analysis");
    ctx->target->setSize(854, 480);

    durProcessing = 0us;
    durRendering = 0us;
    durLoop = 0us;
}

void ContextManager::start()
{
    loadSettings();

    ctx->audio->openPlaybackStream(nullptr);
    ctx->audio->startPlaybackStream();
    
    ctx->audio->openCaptureStream(nullptr);
    ctx->audio->startCaptureStream();

    ctx->target->create();

    ctx->freetypeInstance = std::make_unique<Freetype::FTInstance>(ctx->target.get());
    primaryFont = & ctx->freetypeInstance->font("Montserrat.otf");

    ctx->renderer->initialize();
    updateRendererTargetSize();
    loadRenderingParameters(ctx->renderer->getParameters());

    createAudioNodes();
    createAudioIOs();

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(
            [](void *userdata) {
                auto self = static_cast<ContextManager *>(userdata);
                
                self->mainBody();

                if (self->ctx->target->shouldQuit()) {
                    emscripten_cancel_main_loop();
                }
            }, this, 30, 1);
#else
    while (!ctx->target->shouldQuit()) {
        mainBody();
    }
#endif
}

void ContextManager::terminate()
{
    ctx->renderer->terminate();

    ctx->audio->stopPlaybackStream();
    ctx->audio->closePlaybackStream();

    ctx->audio->stopCaptureStream();
    ctx->audio->closeCaptureStream();

    ctx->audio->terminate();

    ctx->target->hide();
    ctx->target->close();
    ctx->target->terminate();
}

void ContextManager::loadSettings()
{
    analysisDuration = 25;
    analysisMaxFrequency = 3700;

    viewMinFrequency = 20;
    viewMaxFrequency = 8000;
    viewMinGain = -45;
    viewMaxGain = +10;
    viewFrequencyScale = Renderer::FrequencyScale::Mel;

    fftLength = 2048;
    fftMaxFrequency = viewMaxFrequency;

    preEmphasisFrequency = 50.0f;
    linPredOrder = 6;

    spectrogramCount = 400;

    numFormantsToRender = 3;
    formantColors = {
        {0.0f,  1.0f,  0.0f},
        {0.86f, 0.78f, 0.24f},
        {1.0f,  0.71f, 0.76f},
    };

#if defined(ANDROID) || defined(__ANDROID__)
    uiFontSize = 11;
#else
    uiFontSize = 16;
#endif

    spectrogramTrack.resize(spectrogramCount);
    pitchTrack.resize(spectrogramCount);
    formantTrack.resize(spectrogramCount);
}

void ContextManager::updateRendererTargetSize()
{
    int w, h;

    ctx->target->getSizeForRenderer(&w, &h);
    ctx->renderer->setDrawableSize(w, h);

    ctx->target->getSize(&w, &h);
    ctx->renderer->setWindowSize(w, h);
}

void ContextManager::loadRenderingParameters(Renderer::Parameters *p)
{
    p->setMinFrequency(viewMinFrequency);
    p->setMaxFrequency(viewMaxFrequency);
    p->setMinGain(viewMinGain);
    p->setMaxGain(viewMaxGain);
    p->setFrequencyScale(viewFrequencyScale);
}

void ContextManager::createAudioNodes()
{
    int captureSampleRate = ctx->captureBuffer->getSampleRate();

    nodes["prereqs"] = std::make_unique<Nodes::Prereqs>(ctx->captureBuffer.get(), analysisDuration, 0);
    nodes["rs_spec"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 2 * fftMaxFrequency);
    nodes["spec"] = std::make_unique<Nodes::Spectrum>(fftLength);
    nodes["tail"] = std::make_unique<Nodes::Tail>(analysisDuration);
    nodes["pitch"] = std::make_unique<Nodes::PitchTracker>(ctx->pitchSolver.get());
    nodes["rs"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 2 * analysisMaxFrequency);
    nodes["preemph"] = std::make_unique<Nodes::PreEmphasis>(preEmphasisFrequency);
    nodes["linpred"] = std::make_unique<Nodes::LinPred>(ctx->linpredSolver.get(), linPredOrder);
    nodes["formant"] = std::make_unique<Nodes::FormantTracker>(ctx->formantSolver.get());
}

void ContextManager::createAudioIOs()
{
    nodeIOs["prereqs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["rs_spec"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["spec"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["tail"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["pitch"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeFrequencies);
    nodeIOs["tail_rs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["rs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["preemph"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["linpred"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeIIRFilter, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["formant"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeFrequencies, Nodes::kNodeIoTypeFrequencies);
}

void ContextManager::propagateAudio()
{
    auto t0 = Clock::now();

    nodes["prereqs"]->as<Nodes::Prereqs>()->setMinimumOutputLength(
            nodes["rs_spec"]->as<Nodes::Resampler>()->getRequiredInputLength(
                nodes["spec"]->as<Nodes::Spectrum>()->getFFTLength()));

    processAudioNode(nullptr, "prereqs");

    processAudioNode("prereqs", "rs_spec");
    processAudioNode("rs_spec", "spec");
    
    processAudioNode("prereqs", "tail");
    processAudioNode("tail", "pitch");

    processAudioNode("prereqs", "rs");
    processAudioNode("rs", "tail");
    processAudioNode("tail", "preemph");
    processAudioNode("preemph", "linpred");
    processAudioNode("linpred", "formant");

    auto t1 = Clock::now();

    durProcessing = duration_cast<microseconds>(t1 - t0);
}

void ContextManager::processAudioNode(const char *in, const std::string& nodeName)
{
    const Nodes::NodeIO **ins = nullptr;    
    if (in != nullptr) {
        ins = const_cast<decltype(ins)>(Nodes::unpack(nodeIOs[in], &ndi));
    }

    Nodes::NodeIO **outs = Nodes::unpack(nodeIOs[nodeName], &ndo);

    nodes[nodeName]->process(ins, outs);
}

void ContextManager::updateNewData()
{
    auto ioSpectrum         = nodeIOs["spec"]   [0]->as<Nodes::IO::AudioSpec>();
    auto ioLinpredFilter    = nodeIOs["linpred"][0]->as<Nodes::IO::IIRFilter>();
    auto ioLinpredSpectrum  = nodeIOs["linpred"][1]->as<Nodes::IO::AudioSpec>();
    auto ioPitch            = nodeIOs["pitch"]  [0]->as<Nodes::IO::Frequencies>();
    auto ioFormantFreqs     = nodeIOs["formant"][0]->as<Nodes::IO::Frequencies>();
    auto ioFormantBands     = nodeIOs["formant"][1]->as<Nodes::IO::Frequencies>();

    int specLength = ioSpectrum->getLength();
    std::vector<std::array<float, 2>> specSlice(specLength);
    for (int i = 0; i < specLength; ++i) {
        specSlice[i][0] = (ioSpectrum->getSampleRate() * i) / (2.0f * specLength);
        specSlice[i][1] = ioSpectrum->getConstData()[i];
    }

    float lpGain = ioLinpredFilter->getFBConstData()[0];
    
    float pitch = -1.0f;
    std::vector<Analysis::FormantData> formants;

    if (lpGain > 0.5e-4f && ioPitch->getLength() > 0) {
        pitch = ioPitch->get(0);

        int formantCount = ioFormantFreqs->getLength();
        formants.resize(formantCount);
        for (int i = 0; i < formantCount; ++i) {
            formants[i] = {
                .frequency = ioFormantFreqs->get(i),
                .bandwidth = ioFormantBands->get(i),
            };
        }
    }

    spectrogramTrack.pop_front();
    spectrogramTrack.push_back(std::move(specSlice));

    pitchTrack.pop_front();
    pitchTrack.push_back(pitch);

    formantTrack.pop_front();
    formantTrack.push_back(std::move(formants));
}

void ContextManager::render()
{
    ctx->renderer->begin();

    Renderer::SpectrogramRenderData specRender;
    for (const auto& [frequency, intensity] : spectrogramTrack.back()) {
        specRender.push_back({frequency, intensity});
    }
    ctx->renderer->renderSpectrogram(specRender, spectrogramCount);

    Renderer::FrequencyTrackRenderData pitchTrackRender;
    for (const auto& x : pitchTrack) {
        if (x > 0)
            pitchTrackRender.push_back(std::make_optional<float>(x));
        else
            pitchTrackRender.emplace_back(std::nullopt);
    }
    ctx->renderer->renderFrequencyTrack(pitchTrackRender, 4.0f, 0.0f, 1.0f, 0.0f);

    std::vector<Renderer::FrequencyTrackRenderData> formantTrackRender(numFormantsToRender);
    for (const auto& formants : formantTrack) {
        int i = 0;
        for (const auto& formant : formants) {
            formantTrackRender[i].push_back(std::make_optional<float>(formant.frequency));
            if (++i >= numFormantsToRender) break;
        }
        for (int j = i; j < numFormantsToRender; ++j)
            formantTrackRender[j].emplace_back(std::nullopt);
    }
    for (int i = 0; i < numFormantsToRender; ++i) {
        const auto [r, g, b] = formantColors[i];
        ctx->renderer->renderFrequencyTrack(formantTrackRender[i], 6.0f, r, g, b);
    }

    if (durLoop > 0us) {
        std::stringstream ss;
        ss << "Loop cycle took " << (durLoop.count() / 1000.0f) << " ms";
        ss.flush();
        ctx->renderer->renderText(
                primaryFont->with(uiFontSize),
                ss.str(),
                20,
                20,
                1.0f, 0.5f, 1.0f);

        float processingFrac = (float) durProcessing.count() / (float) durLoop.count();
        float renderingFrac = (float) durRendering.count() / (float) durLoop.count();

        ss.str("");
        ss << "- Processing: " << std::round(100 * processingFrac) << "%";
        ss.flush();
        ctx->renderer->renderText(
                primaryFont->with(uiFontSize),
                ss.str(),
                20,
                20+uiFontSize+5,
                1.0f, 0.5f, 1.0f);

        ss.str("");
        ss << "- Rendering: " << std::round(100 * renderingFrac) << "%";
        ss.flush();
        ctx->renderer->renderText(
                primaryFont->with(uiFontSize),
                ss.str(),
                20,
                20+2*(uiFontSize+5),
                1.0f, 0.5f, 1.0f);
    }

    ctx->renderer->end();
}

void ContextManager::mainBody()
{
    auto t0 = Clock::now();

    ctx->target->processEvents();

    if (ctx->target->sizeChanged()) {
        updateRendererTargetSize();
    }

    //ctx->playbackQueue->pushIfNeeded(nullptr);

    if (ctx->audio->needsTicking()) {
        ctx->audio->tickAudio();
    }

    propagateAudio();

    updateNewData();

    auto tr0 = Clock::now();

    render();

    auto tr1 = Clock::now();

    durRendering = duration_cast<microseconds>(tr1 - tr0);

    auto t1 = Clock::now();

    durLoop = duration_cast<microseconds>(t1 - t0);
}
