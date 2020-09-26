#include "contextmanager.h"
#include "contextbuilder.h"
#include "../synthesis/synthesis.h"

#ifdef __EMSCRIPTEN__
extern const char *EMSCRIPTEN_CANVAS_NAME;
#endif

#include <iostream>
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

#if defined(ANDROID) || defined(__ANDROID__)
    Target::SDL2::prepareAssets();
#endif

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

    createRenderingContexts({
        {"Spectrogram", &ContextManager::renderSpectrogram, &ContextManager::eventSpectrogram},
        {"FFT spectrum", &ContextManager::renderFFTSpectrum, &ContextManager::eventFFTSpectrum},
        {"Oscilloscope", &ContextManager::renderOscilloscope, &ContextManager::eventOscilloscope},
        {"Settings", &ContextManager::renderSettings, &ContextManager::eventSettings},
    });

    primaryFont = & ctx->freetypeInstance->font("Montserrat.otf");

    initSettingsUI();

#if defined(ANDROID) || defined(__ANDROID__)
    initAndroidUI();

    auto& rctx = ctx->renderingContexts["main"];
    rctx.target->initialize(); 
    rctx.target->setTitle("Speech analysis for Android");
    rctx.target->create();
    rctx.renderer->initialize();
    updateRendererTargetSize(rctx);
    updateRendererParameters(rctx);
    selectedViewName = "Spectrogram";
#else
    for (const auto& [name, info] : renderingContextInfos) {
        auto& rctx = ctx->renderingContexts[name];
        rctx.target->initialize();
        rctx.target->setTitle("Speech analysis - " + info.name); 
        rctx.target->setSize(640, 480);
#ifdef __EMSCRIPTEN__
        std::string canvasIdWithSharp = "#" + info.canvasId;
        EMSCRIPTEN_CANVAS_NAME = canvasIdWithSharp.c_str();
#endif
        rctx.target->create();
        rctx.renderer->initialize();
        updateRendererTargetSize(rctx);
        updateRendererParameters(rctx);
#ifdef __EMSCRIPTEN__
        saveModuleCtx(info.canvasId);
#endif
        if (name == "Settings") {
            rctx.target->hide();
#ifdef __EMSCRIPTEN__
            EM_ASM({
                document.getElementById("popup").classList.replace("visible", "hidden");
            });
#endif
        }
    }
#endif

#ifdef __EMSCRIPTEN__
    EM_ASM({
        setWindowTitle('Speech analysis for Web');
    });
#endif

    createAudioNodes();
    createAudioIOs();

    endLoop = false;
    isPaused = false;
    displayLpSpec = false;
    useFrameCursor = false;
    isNoiseOn = false;

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
    emscripten_set_main_loop_arg(
            [](void *userdata) {
                auto self = static_cast<ContextManager *>(userdata);
                
                self->mainBody(true);
            }, this, 0, 1);
#else
    while (!endLoop) {
        mainBody();

        // Locked to ~60 fps
        if (durLoop < 16.67ms) {
            std::this_thread::sleep_for(16.67ms - durLoop);
        }
    }
#endif
}

void ContextManager::terminate()
{
    ctx->freetypeInstance.reset();

#if defined(ANDROID) || defined(__ANDROID__)
    auto& rctx = ctx->renderingContexts["main"];
#else
    for (const auto& [name, info] : renderingContextInfos) {
#ifdef __EMSCRIPTEN__
        changeModuleCanvas(info.canvasId);
#endif
        auto& rctx = ctx->renderingContexts[name];
#endif
        rctx.renderer->terminate();
        rctx.target->hide();
        rctx.target->close();
        rctx.target->terminate();
#if ! ( defined(ANDROID) || defined(__ANDROID__) )
    }
#endif

    ctx->audio->stopPlaybackStream();
    ctx->audio->closePlaybackStream();

    ctx->audio->stopCaptureStream();
    ctx->audio->closeCaptureStream();

    ctx->audio->terminate();
}

#if defined(ANDROID) || defined(__ANDROID__)
void ContextManager::selectView(const std::string& name)
{
    selectedViewName = name;
}
#endif

void ContextManager::loadSettings()
{
    outputGain = 0;
    
    analysisDuration = 18;
    analysisMaxFrequency = 5300;

    viewMinFrequency = 1;
    viewMaxFrequency = 6000;
    viewMinGain = -75;
    viewMaxGain = +15;
    viewFrequencyScale = Renderer::FrequencyScale::Mel;

    fftLength = 2048;
    fftMaxFrequency = viewMaxFrequency;

    preEmphasisFrequency = 500.0f;
    linPredOrder = 10;

    spectrogramCount = 600;

    numFormantsToRender = 4;
    formantColors = {
        {0.0f,  1.0f,  0.0f},
        {0.57f, 0.93f, 0.57f},
        {1.0f,  0.0f,  0.0f},
        {1.0f,  0.5f,  1.0f},
    };

#if defined(ANDROID) || defined(__ANDROID__)
    uiFontSize = 10;
#else
    uiFontSize = 14;
#endif

    spectrogramTrack.resize(spectrogramCount);
    lpSpecTrack.resize(spectrogramCount);
    pitchTrack.resize(spectrogramCount);
    formantTrack.resize(spectrogramCount);
    soundTrack.resize(spectrogramCount);
    glotTrack.resize(spectrogramCount);
}

void ContextManager::updateRendererTargetSize(RenderingContext& rctx)
{
    int w, h;

    rctx.target->getSizeForRenderer(&w, &h);
    rctx.renderer->setDrawableSize(w, h);

    rctx.target->getSize(&w, &h);
    rctx.renderer->setWindowSize(w, h);
}

void ContextManager::updateRendererParameters(RenderingContext& rctx)
{
    auto p = rctx.renderer->getParameters();

    p->setMinFrequency(viewMinFrequency);
    p->setMaxFrequency(viewMaxFrequency);
    p->setMinGain(viewMinGain);
    p->setMaxGain(viewMaxGain);
    p->setFrequencyScale(viewFrequencyScale);
}

void ContextManager::updateAllRendererParameters()
{
#if defined(ANDROID) || defined(__ANDROID__)
    auto& rctx = ctx->renderingContexts["main"];
#else
    for (auto& [name, rctx] : ctx->renderingContexts)
#endif
    {
        updateRendererParameters(rctx);
    }
}

void ContextManager::createRenderingContexts(const std::initializer_list<RenderingContextInfo>& infos)
{
    RenderingContextBuilder<Target::SDL2> builder(ctx->rendererType);
#if defined(ANDROID) || defined(__ANDROID__)
    ctx->renderingContexts["main"] = builder.build();
#endif

    for (const auto& info : infos) {
#if ! ( defined(ANDROID) || defined(__ANDROID__) )
        ctx->renderingContexts[info.name] = builder.build();
#endif

        renderingContextInfos[info.name] = info;
        
#ifdef __EMSCRIPTEN__
        std::string canvasId = info.name;
        std::transform(canvasId.begin(), canvasId.end(), canvasId.begin(),
                [](char c) { return c == ' ' ? '_' : std::tolower(c); });
        renderingContextInfos[info.name].canvasId = canvasId;
#endif
    }
}

void ContextManager::createAudioNodes()
{
    int captureSampleRate = ctx->captureBuffer->getSampleRate();

    nodes["prereqs"] = std::make_unique<Nodes::Prereqs>(ctx->captureBuffer.get(), analysisDuration, 256);
    nodes["rs_spec"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 2 * fftMaxFrequency);
    nodes["spec"] = std::make_unique<Nodes::Spectrum>(fftLength);
    nodes["rs_2"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 16000);
    nodes["tail_2"] = std::make_unique<Nodes::Tail>(analysisDuration);
    nodes["pitch"] = std::make_unique<Nodes::PitchTracker>(ctx->pitchSolver.get());
    nodes["invglot"] = std::make_unique<Nodes::InvGlot>(ctx->invglotSolver.get());
    nodes["linpred_2"] = std::make_unique<Nodes::LinPred>(ctx->linpredSolver.get(), 2 * 16000 / 2000 + 4);
    nodes["rs"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 2 * analysisMaxFrequency);
    nodes["tail_rs"] = std::make_unique<Nodes::Tail>(analysisDuration);
    nodes["preemph"] = std::make_unique<Nodes::PreEmphasis>(preEmphasisFrequency);
    nodes["linpred"] = std::make_unique<Nodes::LinPred>(ctx->linpredSolver.get(), linPredOrder);
    nodes["formant"] = std::make_unique<Nodes::FormantTracker>(ctx->formantSolver.get());
}

void ContextManager::createAudioIOs()
{
    nodeIOs["prereqs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["rs_spec"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["spec"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["rs_2"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["tail_2"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["pitch"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeFrequencies);
    nodeIOs["invglot"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["linpred_2"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeIIRFilter, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["rs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["tail_rs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["preemph"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["linpred"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeIIRFilter, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["formant"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeFrequencies, Nodes::kNodeIoTypeFrequencies);
}

void ContextManager::updateNodeParameters()
{
    nodes["rs_spec"]->as<Nodes::Resampler>()->setOutputSampleRate(2 * viewMaxFrequency);
    nodes["rs"]->as<Nodes::Resampler>()->setOutputSampleRate(2 * analysisMaxFrequency);
    nodes["linpred"]->as<Nodes::LinPred>()->setOrder(linPredOrder);
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
    
    processAudioNode("prereqs", "rs_2");
    processAudioNode("rs_2", "pitch");
    processAudioNode("rs_2", "tail_2");
    processAudioNode("tail_2", "invglot");
    processAudioNode("tail_2", "preemph");
    processAudioNode("preemph", "linpred_2");

    processAudioNode("prereqs", "rs");
    processAudioNode("rs", "tail_rs");
    processAudioNode("tail_rs", "preemph");
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
    auto ioTail2            = nodeIOs["tail_2"] [0]->as<Nodes::IO::AudioTime>();
    auto ioInvglot          = nodeIOs["invglot"][0]->as<Nodes::IO::AudioTime>();

    int specLength = ioSpectrum->getLength();
    std::vector<std::array<float, 2>> specSlice(specLength);
    for (int i = 0; i < specLength; ++i) {
        specSlice[i][0] = (ioSpectrum->getSampleRate() * i) / (2.0f * specLength);
        specSlice[i][1] = ioSpectrum->getConstData()[i];
    }

    int lpSpecLength = ioLinpredSpectrum->getLength();
    std::vector<std::array<float, 2>> lpSpecSlice(lpSpecLength);
    for (int i = 0; i < lpSpecLength; ++i) {
        lpSpecSlice[i][0] = (ioLinpredSpectrum->getSampleRate() * i) / (2.0f * lpSpecLength);
        lpSpecSlice[i][1] = log10(1 + ioLinpredSpectrum->getConstData()[i]);
    }

    float lpGain = ioLinpredFilter->getFFConstData()[0];
    
    float pitch = -1.0f;
    std::vector<Analysis::FormantData> formants;

    if (ioPitch->getLength() > 0) {
        pitch = ioPitch->get(0);
    }

    int formantCount = ioFormantFreqs->getLength();
    formants.resize(formantCount);
    for (int i = 0; i < formantCount; ++i) {
        formants[i] = {
            .frequency = ioFormantFreqs->get(i),
            .bandwidth = ioFormantBands->get(i),
        };
    }

    std::vector<float> sound(ioTail2->getConstData(), ioTail2->getConstData() + ioTail2->getLength());
    std::vector<float> glot(ioInvglot->getConstData(), ioInvglot->getConstData() + ioInvglot->getLength());

    spectrogramTrack.pop_front();
    spectrogramTrack.push_back(std::move(specSlice));

    lpSpecTrack.pop_front();
    lpSpecTrack.push_back(std::move(lpSpecSlice));

    pitchTrack.pop_front();
    pitchTrack.push_back(pitch);

    formantTrack.pop_front();
    formantTrack.push_back(std::move(formants));

    soundTrack.pop_front();
    soundTrack.push_back(std::move(sound));

    glotTrack.pop_front();
    glotTrack.push_back(std::move(glot));
}

void ContextManager::generateAudio(float *x, int length)
{
    if (outputGain > 1e-6) {
        auto lpc = nodeIOs["linpred_2"][0]->as<Nodes::IO::IIRFilter>();

        float b = 0.07;
        std::vector<float> a(lpc->getFBConstData(), lpc->getFBConstData() + lpc->getFBOrder());
        a.insert(a.begin(), 1.0f);

        const float sampleRate = ctx->playbackQueue->getInSampleRate();

        static Audio::Resampler resampler(lpc->getSampleRate(), sampleRate);
        resampler.setRate(lpc->getSampleRate(), sampleRate);

        int inlen = resampler.getRequiredInLength(length);

        static float lastNoise = 0.0f;
        auto noise = Synthesis::brownNoise(inlen, lastNoise);
        lastNoise = noise.back();

        static std::deque<float> memoryOut(30, 0.0f);
        auto filtNoise = Synthesis::filter(b, a, noise, memoryOut);

        auto output = resampler.process(filtNoise.data(), inlen);
      
        for (int i = 0; i < std::min<int>(length, output.size()); ++i) {
            x[i] = outputGain * output[i];
        }
    }
}

void ContextManager::mainBody(bool processEvents)
{
auto t0 = Clock::now();

#if defined(ANDROID) || defined(__ANDROID__)
    auto& rctx = ctx->renderingContexts["main"];
        const auto& name = selectedViewName;
        const auto& info = renderingContextInfos[name];
#define break 
#endif

    if (processEvents) {
#if ! ( defined(ANDROID) || defined(__ANDROID__) )
        for (const auto& [name, info] : renderingContextInfos) {
#ifdef __EMSCRIPTEN__
            changeModuleCanvas(info.canvasId);
#endif
            auto& rctx = ctx->renderingContexts[name];
#endif

            rctx.target->processEvents();

            if (rctx.target->isVisible()) {
#ifndef __EMSCRIPTEN__
                if (rctx.target->shouldQuit()) {
                    endLoop = true;
                    break;
                }
#endif

                if ((rctx.target->isKeyPressedOnce(SDL_SCANCODE_ESCAPE)
                        || rctx.target->shouldClose())
#ifdef __EMSCRIPTEN__
                        && name == "Settings"
#endif    
                    ) {
                    if (name == "Spectrogram") {
                        endLoop = true;
                        break;
                    }
                    else {
                        rctx.target->hide();
#ifdef __EMSCRIPTEN__
                        EM_ASM({
                            document.getElementById("popup").classList.replace("visible", "hidden");
                        });
#endif
                    }
                }

                if (name != "Settings") {
                    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_S)) {
#ifdef __EMSCRIPTEN__
                        EM_ASM({
                            document.getElementById("popup").classList.replace("hidden", "visible");
                            var canvas = document.getElementById("settings");
                            canvas.width = canvas.clientWidth;
                            canvas.height = canvas.clientHeight;
                        });
#endif
                        ctx->renderingContexts["Settings"].target->show();
                    }
                }
                 
                if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_P)) {
                    isPaused = !isPaused;
                }

                if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_N)) {
                    isNoiseOn = !isNoiseOn;
                }

                if (rctx.target->sizeChanged()) {
                    updateRendererTargetSize(rctx);
                }

                (this->*info.eventCallback)(rctx);
            }
#if defined(ANDROID) || defined(__ANDROID__)
#   undef break
            eventAndroidCommon(rctx);
#else
        }
#endif
    }

    if (endLoop) {
        return;
    }

    if (ctx->audio->needsTicking()) {
        ctx->audio->tickAudio();
    }

    if (!isPaused) {
        propagateAudio();
        updateNewData();
    }

    if (isNoiseOn) {
        outputGain = 0.5 * outputGain + 0.5;
    }
    else {
        outputGain = 0.5 * outputGain;
    }

    ctx->playbackQueue->pushIfNeeded(this);

    auto tr0 = Clock::now();

#if ! ( defined(ANDROID) || defined(__ANDROID__) )
    for (const auto& [name, info] : renderingContextInfos) {
#ifdef __EMSCRIPTEN__
        changeModuleCanvas(info.canvasId);
#endif

        auto& rctx = ctx->renderingContexts[name];
#endif

        if (rctx.target->isVisible()) {
            rctx.renderer->begin();
            (this->*info.renderCallback)(rctx);
#if defined(ANDROID) || defined(__ANDROID__)
            renderAndroidCommon(rctx);
#endif
            rctx.renderer->end(); 
        }
#if ! (defined(ANDROID) || defined(__ANDROID__) ) 
    }
#endif

    auto tr1 = Clock::now();

    durRendering = duration_cast<microseconds>(tr1 - tr0);

    auto t1 = Clock::now();

    durLoop = duration_cast<microseconds>(t1 - t0);
}

#ifdef __EMSCRIPTEN__
void ContextManager::changeModuleCanvas(const std::string& id)
{
    static std::string str;
    str = "#" + id;
    EMSCRIPTEN_CANVAS_NAME = str.c_str();
    EM_ASM({
        var canvasId = UTF8ToString($0);
        Module['canvas'] = document.getElementById(canvasId);
        EGL['context'] = Module['ctx-' + canvasId];
    }, id.c_str());
}

void ContextManager::saveModuleCtx(const std::string& id)
{
    EM_ASM({
        Module['ctx-' + UTF8ToString($0)] = EGL['context'];
    }, id.c_str());
}
#endif
