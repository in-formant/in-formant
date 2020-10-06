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
      pipeline(this->ctx->captureBuffer.get()),
      synth(this->ctx->playbackQueue.get())
{
    this->ctx->playbackQueue->setCallback(App::Synthesizer::audioCallback);
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
        {"Spectrogram",  &ContextManager::renderSpectrogram,  &ContextManager::eventSpectrogram},
        {"FFT spectrum", &ContextManager::renderFFTSpectrum,  &ContextManager::eventFFTSpectrum},
        {"Oscilloscope", &ContextManager::renderOscilloscope, &ContextManager::eventOscilloscope},
        {"Settings",     &ContextManager::renderSettings,     &ContextManager::eventSettings},
#if ! ( defined(__EMSCRIPTEN__) || defined(ANDROID) || defined(__ANDROID__) )
        {"Synthesizer",  &ContextManager::renderSynth,        &ContextManager::eventSynth},
#endif
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
#ifndef __EMSCRIPTEN__
        rctx.target->setSize(640, 480);
#else
        rctx.target->setSize(320, 200);
        std::string canvasIdWithSharp = "#" + info.canvasId;
        EMSCRIPTEN_CANVAS_NAME = canvasIdWithSharp.c_str();
#endif
        if (name == "Settings") {
#ifdef __EMSCRIPTEN__
            EM_ASM({
                document.getElementById("popup").classList.replace("hidden", "visible");
            });
#endif
        }
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

    updateNodeParameters();
    pipeline.initialize();
    synth.initialize();

    endLoop = false;
    isPaused = false;
    displayLpSpec = false;
    useFrameCursor = false;
    isNoiseOn = false;
    displayFormantTracks = false;
    displayLegends = true;

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(
            [] (void *userdata) {
                auto self = static_cast<ContextManager *>(userdata);
                self->mainBody(true);
            }, this, 0, 1);
#else
    while (!endLoop) {
        mainBody();

        // Locked to ~50 fps
        if (durLoop < 20ms) {
            std::this_thread::sleep_for(20ms - durLoop);
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
    
    analysisDuration = 30;
    analysisMaxFrequency = 5300;

    viewMinFrequency = 1;
    viewMaxFrequency = 6000;
    viewMinGain = -60;
    viewMaxGain = +20;
    viewFrequencyScale = Renderer::FrequencyScale::Mel;

    fftLength = 4096;
    fftMaxFrequency = viewMaxFrequency;

    preEmphasisFrequency = 200.0f;
    
    pitchAndLpSampleRate = 16'000;

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

void ContextManager::updateNodeParameters()
{
    fftMaxFrequency = viewMaxFrequency;

    pipeline.setPitchSolver(ctx->pitchSolver.get());
    pipeline.setInvglotSolver(ctx->invglotSolver.get());
    pipeline.setLinpredSolver(ctx->linpredSolver.get());
    pipeline.setFormantSolver(ctx->formantSolver.get());

    pipeline.setAnalysisDuration(milliseconds(analysisDuration));

    pipeline.setFFTSampleRate(2 * fftMaxFrequency);
    pipeline.setFFTSize(fftLength);

    pipeline.setPreEmphasisFrequency(preEmphasisFrequency);

    pipeline.setPitchAndLpSpectrumSampleRate(pitchAndLpSampleRate);

    pipeline.setFormantSampleRate(2 * analysisMaxFrequency);
    pipeline.setFormantLpOrder(linPredOrder);
}

void ContextManager::updateWithNextFrame()
{
    spectrogramTrack.pop_front();
    spectrogramTrack.push_back(pipeline.getFFTSlice());

    lpSpecTrack.pop_front();
    lpSpecTrack.push_back(pipeline.getLpSpectrumSlice());

    pitchTrack.pop_front();
    pitchTrack.push_back(pipeline.getPitch());

    formantTrack.pop_front();
    formantTrack.push_back(pipeline.getFormants());

    soundTrack.pop_front();
    soundTrack.push_back(pipeline.getSound());

    glotTrack.pop_front();
    glotTrack.push_back(pipeline.getGlottalFlow());
    
    synth.setFormants(pipeline.getFormants(), 2 * analysisMaxFrequency);
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
                            var popup = document.getElementById("popup");
                            popup.classList.replace("hidden", "visible");
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

#ifdef __EMSCRIPTEN__
                if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_N)) {
                    isNoiseOn = !isNoiseOn;
                }
#endif

                if (rctx.target->sizeChanged()) {
                    updateRendererTargetSize(rctx);
                }

                (this->*info.eventCallback)(rctx);
            }
#if ! ( defined(ANDROID) || defined(__ANDROID__) )
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
        pipeline.processAll();
        updateWithNextFrame();
    }

#ifdef __EMSCRIPTEN__
    synth.setMasterGain(isNoiseOn ? 1 : 0);
    synth.setNoiseGain(isNoiseOn ? 0.3 : 0);
#endif

    ctx->playbackQueue->pushIfNeeded(&synth);

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
#if defined(ANDROID) || defined(__ANDROID__)
            renderAndroidCommonBefore(rctx);
#endif
            (this->*info.renderCallback)(rctx);
#if defined(ANDROID) || defined(__ANDROID__)
            renderAndroidCommonAfter(rctx);
#endif
            rctx.renderer->end(); 
        }
#if ! (defined(ANDROID) || defined(__ANDROID__) ) 
    }
#endif

    if (processEvents) {
#if defined(ANDROID) || defined(__ANDROID__)
#   undef break
        eventAndroidCommon(rctx);
#endif
    }

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
