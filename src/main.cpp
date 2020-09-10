#include "modules/modules.h"
#include "nodes/nodes.h"
#include "analysis/analysis.h"
#include "backtrace/backtrace.h"
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>
#include <csignal>
#include <thread>

using namespace Module;
using namespace std::literals::chrono_literals;
using Clock = std::chrono::steady_clock;

const auto testLoopInterval = 33ms;

constexpr int captureSampleRate = 48000;
constexpr int captureDuration = 200;

constexpr int playbackBlockMinDuration = 5;
constexpr int playbackBlockMaxDuration = 200;
constexpr int playbackDuration = 1000;
constexpr int playbackSampleRate = 96000;

constexpr int testToneFrequency = 200;

constexpr int analysisDuration = 25;
constexpr int analysisSampleRate = 2 * 3700;

constexpr int fftMaxFrequency = 8000;
constexpr int fftLength = 2048;

constexpr float preEmphasisFrequency = 2000.0f;
constexpr int linPredOrder = 6;

constexpr int spectrogramCount = 400;

constexpr int numFormantsRender = 3;
constexpr float formantColors[numFormantsRender][3] = {
    {0.0f,  1.0f,  0.0f},
    {0.86f, 0.78f, 0.24f},
    {1.0f,  0.71f, 0.76f},
};

#if defined(ANDROID) || defined(__ANDROID__)
constexpr int uiFontSize = 10;
#else
constexpr int uiFontSize = 16;
#endif

static std::atomic_bool signalCaught(false);
static std::atomic_int signalStatus;

static void signalHandler(int signal) {
    signalCaught = true;
    signalStatus = signal;

    switch (signal) {
    case SIGTERM:
        std::cout << "Caught signal SIGTERM" << std::endl;
        break;
    case SIGINT:
        std::cout << "Caught signal SIGINT" << std::endl;
        break;
    }

    exit(EXIT_SUCCESS);
}

static void playbackCallback(float *out, int len, void *data) {
    auto pTime = static_cast<int *>(data);
    
    // Simple sine wave output for testing purposes.
    for (int i = 0; i < len; ++i) {
        out[i] = 0.5 * std::sin(2 * M_PI * (*pTime) * testToneFrequency / playbackSampleRate);
        (*pTime)++;
    }
}

int start_logger(const char *app_name);

int main(int argc, char **argv)
{
    start_logger("SpeechAnalysis");

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);
    registerCrashHandler();
    
    constexpr auto type = Renderer::Type::NanoVG;
    
    std::shared_ptr<Target::AbstractBase> target(nullptr);
    std::shared_ptr<Audio::AbstractBase> audio(nullptr);
    std::shared_ptr<Renderer::AbstractBase> renderer(nullptr);

    std::shared_ptr<Analysis::PitchSolver> pitchSolver(nullptr);
    std::shared_ptr<Analysis::LinpredSolver> linpredSolver(nullptr);
    std::shared_ptr<Analysis::FormantSolver> formantSolver(nullptr);

    target.reset(new Target::SDL2(type));
    target->initialize();
    target->setTitle("Speech analysis");
    target->setSize(854, 480);

    target->create();

    //audio.reset(new Audio::Pulse);
#if defined(ANDROID) || defined(__ANDROID__)
    audio.reset(new Audio::Oboe);
#elif defined(__linux__)
    audio.reset(new Audio::Alsa);
#elif defined(_WIN32) || defined(__APPLE__)
    audio.reset(new Audio::PortAudio);
#endif
    audio->initialize();
    audio->refreshDevices();

    auto captureBuffer = std::make_unique<Audio::Buffer>(
            captureSampleRate, captureDuration);

    auto playbackQueue = std::make_unique<Audio::Queue>(
            playbackBlockMinDuration, playbackBlockMaxDuration,
            playbackDuration, playbackSampleRate,
            playbackCallback);

    audio->setCaptureBuffer(captureBuffer.get());
    audio->setPlaybackQueue(playbackQueue.get());

    audio->openPlaybackStream(nullptr);
    audio->startPlaybackStream();
    
    audio->openCaptureStream(nullptr);
    audio->startCaptureStream();

    std::cout << "Opened playback stream: " << playbackQueue->getOutSampleRate() << std::endl;
    std::cout << "Opened capture stream: " << captureBuffer->getSampleRate() << std::endl;

    if (type == Renderer::Type::OpenGL) {
#ifdef RENDERER_USE_OPENGL
        renderer.reset(new Renderer::OpenGL);
        renderer->setProvider(target->getOpenGLProvider());
#else
        throw std::runtime_error("Main] Requested OpenGL renderer but wasn't compiled with OpenGL or OpenGL ES support.");
#endif
    }

    if (type == Renderer::Type::GLES) {
#ifdef RENDERER_USE_GLES
        renderer.reset(new Renderer::GLES);
        renderer->setProvider(target->getOpenGLProvider());
#else
        throw std::runtime_error("Main] Requested GLES renderer but wasn't compiled with GLES support.");
#endif
    }

    if (type == Renderer::Type::Vulkan) {
#ifdef RENDERER_USE_VULKAN
        renderer.reset(new Renderer::Vulkan);
        renderer->setProvider(target->getVulkanProvider());
#else
        throw std::runtime_error("Main] Requested Vulkan renderer but wasn't compiled with Vulkan support.");
#endif
    }

    if (type == Renderer::Type::SDL2) {
#ifdef RENDERER_USE_SDL2
        renderer.reset(new Renderer::SDL2);
        renderer->setProvider(target->getSDL2Provider());
#else
        throw std::runtime_error("Main] Requested SDL2 renderer but wasn't compiled with SDL2 support.");
#endif
    }

    if (type == Renderer::Type::NanoVG) {
#ifdef RENDERER_USE_NVG
        renderer.reset(new Renderer::NanoVG);
        renderer->setProvider(target->getNvgProvider());
#else
        throw std::runtime_error("Main] Requested NanoVG renderer but wasn't compiled with NanoVG support.");
#endif
    }

    int renderWidth, renderHeight;
    target->getSizeForRenderer(&renderWidth, &renderHeight);
    renderer->setDrawableSize(renderWidth, renderHeight);

    int targetWidth, targetHeight;
    target->getSize(&targetWidth, &targetHeight);
    renderer->setWindowSize(targetWidth, targetHeight);

    renderer->initialize();

    auto renderPar = renderer->getParameters();
    renderPar->setMinFrequency(0.0f);
    renderPar->setMaxFrequency(fftMaxFrequency);
    renderPar->setMinGain(-45.0f);
    renderPar->setMaxGain(10.0f);
    renderPar->setFrequencyScale(Renderer::FrequencyScale::Mel);

    Freetype::FTInstance ft(target.get());
    auto& font = ft.font("Montserrat.otf");

    int sineTime = 0;

    //pitchSolver.reset(new Analysis::Pitch::AMDF_M(60.0f, 2000.0f, 0.4f));
    pitchSolver.reset(new Analysis::Pitch::Yin(0.35f));

    linpredSolver.reset(new Analysis::LP::Burg);

    formantSolver.reset(new Analysis::Formant::SimpleLP);

    Nodes::Prereqs nodePrereqs(captureBuffer.get(), analysisDuration, 0);
    Nodes::Resampler nodeSpectrumResampler(captureBuffer->getSampleRate(), 2 * fftMaxFrequency);
    Nodes::Spectrum nodeSpectrum(fftLength);
    Nodes::Resampler nodeResampler(captureBuffer->getSampleRate(), analysisSampleRate);
    Nodes::Tail nodeTail(analysisDuration);
    Nodes::PitchTracker nodePitch(pitchSolver.get());
    Nodes::PreEmphasis nodePreemph(preEmphasisFrequency);
    Nodes::LinPred nodeLinpred(linpredSolver.get(), linPredOrder);
    Nodes::FormantTracker nodeFormants(formantSolver.get());

    auto ioPrereqs = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    auto ioSpectrumResampler = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    auto ioSpectrum = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioSpec);
    auto ioResampler = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    auto ioTailBeforeResampler = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    auto ioTailAfterResampler = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    auto ioPitch = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeFrequencies);
    auto ioPreemph = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    auto ioLinpred = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeIIRFilter, Nodes::kNodeIoTypeAudioSpec);
    auto ioFormants = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeFrequencies, Nodes::kNodeIoTypeFrequencies);

    Nodes::NodeIO **ndi = nullptr;
    Nodes::NodeIO **ndo = nullptr;

    std::deque<std::vector<std::array<float, 2>>>  spectrogramTrack(spectrogramCount);
    std::deque<float>                              pitchTrack(spectrogramCount);
    std::deque<std::vector<Analysis::FormantData>> formantTrack(spectrogramCount);
     
    while (!target->shouldQuit()) {
        auto tLoopStart = Clock::now();

        target->processEvents();
        
        if (target->sizeChanged()) {
            target->getSizeForRenderer(&renderWidth, &renderHeight);
            renderer->setDrawableSize(renderWidth, renderHeight);

            target->getSize(&targetWidth, &targetHeight);
            renderer->setWindowSize(targetWidth, targetHeight);
        }

        //playbackQueue->pushIfNeeded(&sineTime);

        if (audio->needsTicking()) {
            audio->tickAudio();
        }
       
        nodePrereqs.setMinimumOutputLength(
                nodeSpectrumResampler.getRequiredInputLength(
                    nodeSpectrum.getFFTLength()));

        auto tProcessingStart = Clock::now();

#define in(x) (const_cast<const Nodes::NodeIO **>(Nodes::unpack((x), &ndi)))
#define out(x) (Nodes::unpack((x), &ndo))

        nodePrereqs.process(nullptr, out(ioPrereqs));
        nodeSpectrumResampler.process(in(ioPrereqs), out(ioSpectrumResampler));
        nodeSpectrum.process(in(ioSpectrumResampler), out(ioSpectrum));
        nodeResampler.process(in(ioPrereqs), out(ioResampler));
        nodeTail.process(in(ioPrereqs), out(ioTailBeforeResampler));
        nodePitch.process(in(ioTailBeforeResampler), out(ioPitch));
        nodeTail.process(in(ioResampler), out(ioTailAfterResampler));
        nodePreemph.process(in(ioTailAfterResampler), out(ioPreemph));
        nodeLinpred.process(in(ioPreemph), out(ioLinpred));
        nodeFormants.process(in(ioLinpred), out(ioFormants));

        //std::cout << "    Analysis took  " << (std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - tProcessingStart).count() / 1000.0f) << " ms" << std::endl;;

        auto outSpectrumAudio = ioSpectrum[0]->as<Nodes::IO::AudioSpec>();
        //auto outSpectrumAudio = ioLinpred[1]->as<Nodes::IO::AudioSpec>();
        auto outTailAudio = ioTailBeforeResampler[0]->as<Nodes::IO::AudioTime>();
        auto outPitchFreq = ioPitch[0]->as<Nodes::IO::Frequencies>();
        auto outLinpredFilter = ioLinpred[0]->as<Nodes::IO::IIRFilter>();
        auto outFormantFreqs = ioFormants[0]->as<Nodes::IO::Frequencies>();
        auto outFormantBands = ioFormants[1]->as<Nodes::IO::Frequencies>();

        float lpGain = outLinpredFilter->getFBConstData()[0];

        // Calculate FFT.
        int spectrumOutLength = outSpectrumAudio->getLength();
        std::vector<std::array<float, 2>> specSlice(spectrumOutLength);
        for (int i = 0; i < spectrumOutLength; ++i) {
            specSlice[i][0] = (outSpectrumAudio->getSampleRate() * i) / (2.0f * spectrumOutLength);
            specSlice[i][1] = outSpectrumAudio->getConstData()[i];
        }

        float pitch = -1.0f;
        std::vector<Analysis::FormantData> formantsData;

        if (lpGain > .5e-4f) {
            if (outPitchFreq->getLength() > 0)
                pitch = outPitchFreq->get(0);

            for (int i = 0; i < outFormantFreqs->getLength(); ++i) {
                formantsData.push_back({
                    .frequency = outFormantFreqs->get(i),
                    .bandwidth = outFormantBands->get(i),
                });
            }
        }

        spectrogramTrack.pop_front();
        spectrogramTrack.push_back(specSlice);
        
        pitchTrack.pop_front();
        pitchTrack.push_back(pitch);

        formantTrack.pop_front();
        formantTrack.push_back(formantsData);

        // Send render data to the renderer.
        
        auto tRenderingStart = Clock::now();

        renderer->begin();

        if (false) {
            auto& audio = outTailAudio;
            int length = audio->getLength();

            Module::Renderer::GraphRenderData graph(length);
            for (int i = 0; i < length; ++i) {
                graph[i] = {
                    .x = (float) i / (float) audio->getSampleRate(),
                    .y = audio->getConstData()[i] * 20.0f,
                };
            }
            renderer->renderGraph(graph);
        }

        if (false) {
            Module::Renderer::GraphRenderData graph(specSlice.size());
            for (int i = 0; i < specSlice.size(); ++i) {
                graph[i] = {
                   .x = specSlice[i][0],
                   .y = specSlice[i][1] * 15.0f - 9.8f,
                };
            }
            renderer->renderGraph(graph);
        }

        if (true) {
            Module::Renderer::SpectrogramRenderData specRender;
            for (const auto& point : specSlice) {
                specRender.push_back({
                        .frequency = point[0],
                        .intensity = point[1],
                    });
            }
            renderer->renderSpectrogram(specRender, spectrogramCount);

            Module::Renderer::FrequencyTrackRenderData pitchTrackRender;
            for (const auto& x : pitchTrack) {
                if (x > 0) {
                    pitchTrackRender.push_back(std::make_optional<float>(x));
                }
                else {
                    pitchTrackRender.emplace_back(std::nullopt);
                }
            }
            renderer->renderFrequencyTrack(pitchTrackRender, 4.0f, 0.0f, 1.0f, 1.0f);

            std::array<Module::Renderer::FrequencyTrackRenderData, numFormantsRender> formantTrackRender;
           
            for (const auto& formants : formantTrack) {
                int i = 0;
                for (const auto& formant : formants) {
                    formantTrackRender[i].push_back(std::make_optional<float>(formant.frequency));
                    i++;
                    if (i >= numFormantsRender) break;
                }
                for (int j = i; j < numFormantsRender; ++j) {
                    formantTrackRender[j].emplace_back(std::nullopt);
                }
            }

            for (int i = 0; i < numFormantsRender; ++i) {
                const auto [r, g, b] = formantColors[i];
                renderer->renderFrequencyTrack(formantTrackRender[i], 6.0f, r, g, b);
            }
        }

        auto durLoop = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - tLoopStart);
        std::stringstream ss;
        ss << "Loop cycle took " << (durLoop.count() / 1000.0f) << " ms";
        ss.flush(); 
        renderer->renderText(
                font.with(uiFontSize),
                ss.str(),
                20,
                20,
                1.0f, 0.5f, 1.0f);
       
        renderer->end();

        //std::cout << "    Rendering took " << (std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - tRenderingStart).count() / 1000.0f) << " ms" << std::endl;;
        
        //std::cout << "  Loop cycle took  " << (durLoop.count() / 1000.0f) << " ms" << std::endl;

        if (testLoopInterval - durLoop > 0us) {
            std::this_thread::sleep_for(testLoopInterval - durLoop);
        } else {
            std::cout << "!!Loop cycle took too long (must be less than " << testLoopInterval.count() << " ms)" << std::endl;
        }

        if (signalCaught.load()) {
            break;
        }
    }

    delete[] ndi;
    delete[] ndo;

    renderer->terminate();

    audio->stopPlaybackStream();
    audio->closePlaybackStream();

    audio->stopCaptureStream();
    audio->closeCaptureStream();

    audio->terminate();

    target->hide();
    target->close();
    target->terminate();
   
    return 0;
}

