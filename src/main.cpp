#include "modules/modules.h"
#include "nodes/nodes.h"
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

const auto testLoopInterval = 25ms;

constexpr int captureSampleRate = 48000;
constexpr int captureDuration = 200;

constexpr int playbackBlockMinDuration = 5;
constexpr int playbackBlockMaxDuration = 200;
constexpr int playbackDuration = 1000;
constexpr int playbackSampleRate = 96000;

constexpr int testToneFrequency = 200;

constexpr int analysisDuration = 25;
constexpr int analysisSampleRate = 11025;

constexpr int fftMaxFrequency = 8000;
constexpr int fftLength = 2048;

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
    
    constexpr auto type = Renderer::Type::OpenGL;
    
    std::shared_ptr<Target::AbstractBase> target(nullptr);
    std::shared_ptr<Audio::AbstractBase> audio(nullptr);
    std::shared_ptr<Renderer::AbstractBase> renderer(nullptr);

    target.reset(new Target::SDL2(type));
    target->initialize();
    target->setTitle(std::string("Speech analysis - ") + (type == Renderer::Type::Vulkan ? "Vulkan" : "OpenGL"));
    target->setSize(500, 500);

    target->create();
    target->show();

    //audio.reset(new Audio::Pulse);
    //audio.reset(new Audio::Alsa);
    audio.reset(new Audio::PortAudio);
    //audio.reset(new Audio::Oboe);
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

    int renderWidth, renderHeight;
    target->getSizeForRenderer(&renderWidth, &renderHeight);
    renderer->setDrawableSize(renderWidth, renderHeight);

    renderer->initialize();

    int sineTime = 0;

    Nodes::Prereqs nodePrereqs(captureBuffer.get(), analysisDuration, 0);
    Nodes::Resampler nodeSpectrumResampler(captureBuffer->getSampleRate(), 2 * fftMaxFrequency);
    Nodes::Spectrum nodeSpectrum(fftLength);
    Nodes::Resampler nodeResampler(captureBuffer->getSampleRate(), analysisSampleRate);

    auto outPrereqs = Nodes::makeNodeIO(Nodes::kNodeIoTypeAudioTime);
    auto outSpectrumResampler = Nodes::makeNodeIO(Nodes::kNodeIoTypeAudioTime);
    auto outSpectrum = Nodes::makeNodeIO(Nodes::kNodeIoTypeAudioSpec);
    auto outResampler = Nodes::makeNodeIO(Nodes::kNodeIoTypeAudioTime);

    int spectrogramCount = 200;
    std::deque<std::vector<std::array<float, 2>>> spectrogram(spectrogramCount);
    auto spectrogramView = std::make_unique<float **[]>(spectrogramCount);
             
    while (!target->shouldQuit()) {
        auto tLoopStart = Clock::now();

        target->processEvents();
        
        if (target->sizeChanged()) {
            int renderWidth, renderHeight;
            target->getSizeForRenderer(&renderWidth, &renderHeight);
            renderer->setDrawableSize(renderWidth, renderHeight);
        }

        //playbackQueue->pushIfNeeded(&sineTime);

        if (audio->needsTicking()) {
            audio->tickAudio();
        }
       
        nodePrereqs.setMinimumOutputLength(
                nodeSpectrumResampler.getRequiredInputLength(
                    nodeSpectrum.getFFTLength()));

        auto tProcessingStart = Clock::now();

        nodePrereqs.process(nullptr, outPrereqs.get());
        nodeSpectrumResampler.process(outPrereqs.get(), outSpectrumResampler.get());
        nodeSpectrum.process(outSpectrumResampler.get(), outSpectrum.get());
        nodeResampler.process(outPrereqs.get(), outResampler.get());

        std::cout << "    Analysis took  " << (std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - tProcessingStart).count() / 1000.0f) << " ms" << std::endl;;

        auto outPrereqsAudio = outPrereqs->as<Nodes::IO::AudioTime>();
        auto outResamplerAudio = outResampler->as<Nodes::IO::AudioTime>();
        auto outSpectrumResamplerAudio = outSpectrumResampler->as<Nodes::IO::AudioTime>();
        auto outSpectrumAudio = outSpectrum->as<Nodes::IO::AudioSpec>();

        // Calculate FFT.
        int spectrumOutLength = outSpectrumAudio->getLength();
        std::vector<std::array<float, 2>> specSlice(spectrumOutLength);
        for (int i = 0; i < spectrumOutLength; ++i) {
            specSlice[i][0] = (outSpectrumAudio->getSampleRate() * (i + 0.5f)) / (2.0f * spectrumOutLength);
            specSlice[i][1] = outSpectrumAudio->getConstData()[i];
        }
        spectrogram.pop_front();
        spectrogram.push_back(specSlice);

        // Send render data to the renderer.
        
        auto tRenderingStart = Clock::now();

        renderer->begin();
        renderer->clear();

        if (false) {
            //auto& audio = outPrereqsAudio;
            auto& audio = outResamplerAudio;

            int length = (analysisDuration * audio->getSampleRate()) / 1000;

            auto times = std::make_unique<float[]>(length);
            auto values = std::make_unique<float[]>(length);
            for (int i = 0; i < length; ++i) {
                times[i] = (float) i / (float) audio->getSampleRate();
                values[i] = audio->getConstData()[i] * 20.0f;
            }
            renderer->renderGraph(times.get(), values.get(), length);
        }

        if (false) {
            const auto& slice = spectrogram.back();
            auto freqs = std::make_unique<float[]>(slice.size());
            auto gains = std::make_unique<float[]>(slice.size());
            for (int i = 0; i < slice.size(); ++i) {
                freqs[i] = slice[i][0];
                gains[i] = slice[i][1] * 0.5f - 9.8f;
            }
            renderer->renderGraph(freqs.get(), gains.get(), slice.size());
        }

        if (true) {
            std::vector<std::vector<std::array<float, 2>>> spectrogramView(spectrogramCount);
            std::vector<size_t> lengths(spectrogramCount);

            for (int i = 0; i < spectrogramCount; ++i) {
                lengths[i] = spectrogram[i].size();
                spectrogramView[i].resize(lengths[i]);
                for (int j = 0; j < lengths[i]; ++j) {
                    spectrogramView[i][j] = spectrogram[i][j];
                }
            }

            auto view = std::make_unique<float **[]>(spectrogramCount);
            std::vector<std::unique_ptr<float *[]>> viewSlices;

            for (int i = 0; i < spectrogramCount; ++i) {
                auto viewSlice = std::make_unique<float *[]>(lengths[i]);

                for (int j = 0; j < lengths[i]; ++j) {
                    viewSlice[j] = spectrogramView[i][j].data();
                }

                view[i] = viewSlice.get();
                viewSlices.push_back(std::move(viewSlice));
            }

            renderer->renderSpectrogram(view.get(), lengths.data(), spectrogramCount);
        }

        renderer->end();

        std::cout << "    Rendering took " << (std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - tRenderingStart).count() / 1000.0f) << " ms" << std::endl;;

        auto durLoop = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - tLoopStart);

        std::cout << "  Loop cycle took  " << (durLoop.count() / 1000.0f) << " ms" << std::endl;

        if (testLoopInterval - durLoop > 0us) {
            std::this_thread::sleep_for(testLoopInterval - durLoop);
        } else {
            std::cout << "!!Loop cycle took too long (must be less than " << testLoopInterval.count() << " ms)" << std::endl;
        }

        if (signalCaught.load()) {
            break;
        }
    }
    
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

