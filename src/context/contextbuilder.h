#ifndef MAIN_CONTEXT_BUILDER_H
#define MAIN_CONTEXT_BUILDER_H

#include <sstream>

#include "context.h"

namespace Main {

    using namespace Module;

    template <typename TargetType>
    class RenderingContextBuilder {
    public:
        RenderingContextBuilder(Renderer::Type rendererType)
            : rendererType(rendererType) {
        }

        RenderingContext build() {
            RenderingContext rctx;
            rctx.target = std::make_unique<TargetType>(rendererType);
            checkBuildRenderer(rctx);
            return rctx;
        }
       
        void throwRendererError(const std::string& name) {
            std::stringstream ss;
            ss << "Main] Requested " << name << " renderer but wasn't compiled with " << name << " support";
            ss.flush();
            throw std::runtime_error(ss.str());
        }

        template<typename RendererType, typename ProviderFunc>
        void buildRenderer(RenderingContext& rctx, ProviderFunc provider) {
            rctx.renderer = std::make_unique<RendererType>();
            rctx.renderer->setProvider((rctx.target.get()->*provider)());
        }

        void checkBuildRenderer(RenderingContext& ctx) {
            switch (rendererType) {
            case Renderer::Type::OpenGL:
#if ! RENDERER_USE_OPENGL
                throwRendererError("OpenGL");
#else
                buildRenderer<Renderer::OpenGL>(ctx, &Target::AbstractBase::getOpenGLProvider);
                break;
#endif
            
            case Renderer::Type::GLES:
#if ! RENDERER_USE_GLES
                throwRendererError("OpenGL ES");
#else
                buildRenderer<Renderer::GLES>(ctx, &Target::AbstractBase::getOpenGLProvider);
                break;
#endif

            case Renderer::Type::Vulkan:
#if ! RENDERER_USE_VULKAN
                throwRendererError("Vulkan");
#else
                buildRenderer<Renderer::Vulkan>(ctx, &Target::AbstractBase::getVulkanProvider);
                break;
#endif

            case Renderer::Type::SDL2:
#if ! RENDERER_USE_SDL2
                throwRendererError("SDL2");
#else
                buildRenderer<Renderer::SDL2>(ctx, &Target::AbstractBase::getSDL2Provider);
                break;
#endif

            case Renderer::Type::NanoVG:
#if ! RENDERER_USE_NVG
                throwRendererError("NanoVG");
#else
                buildRenderer<Renderer::NanoVG>(ctx, &Target::AbstractBase::getNvgProvider);
                break;
#endif
            }
        }
    
    private:
        Renderer::Type rendererType;
    };

    template <typename AudioType, typename TargetType, Renderer::Type rendererType>
    class ContextBuilder {
    public:
        std::unique_ptr<Context> build() {
            auto ctx = std::make_unique<Context>();
            ctx->rendererType = rendererType;

            ctx->audio = std::make_unique<AudioType>();

            ctx->captureBuffer = std::make_unique<Audio::Buffer>(
                    captureSampleRate, captureDuration);

            ctx->playbackQueue = std::make_unique<Audio::Queue>(
                    playbackBlockMinDuration, playbackBlockMaxDuration,
                    playbackDuration, playbackSampleRate,
                    playbackCallback);

            ctx->audio->setCaptureBuffer(ctx->captureBuffer.get());
            ctx->audio->setPlaybackQueue(ctx->playbackQueue.get());

            ctx->freetypeInstance = std::make_unique<Freetype::FTInstance>();

            ctx->pitchSolver.reset(pitchSolver);
            ctx->linpredSolver.reset(linpredSolver);
            ctx->formantSolver.reset(formantSolver);
            ctx->invglotSolver.reset(invglotSolver);
            
            return ctx;
        }

        ContextBuilder& setPitchSolver(Analysis::PitchSolver *o) {
            pitchSolver = o;
            return *this;
        }

        ContextBuilder& setLinpredSolver(Analysis::LinpredSolver *o) {
            linpredSolver = o;
            return *this;
        }

        ContextBuilder& setFormantSolver(Analysis::FormantSolver *o) {
            formantSolver = o;
            return *this;
        }

        ContextBuilder& setInvglotSolver(Analysis::InvglotSolver *o) {
            invglotSolver = o;
            return *this;
        }

        ContextBuilder& setCaptureSampleRate(int o) {
            captureSampleRate = o;
            return *this;
        }

        ContextBuilder& setCaptureDuration(const std::chrono::milliseconds& o) {
            captureDuration = o.count();
            return *this;
        }

        ContextBuilder& setPlaybackBlockDuration(const std::chrono::milliseconds& min,
                                                 const std::chrono::milliseconds& max) {
            playbackBlockMinDuration = min.count();
            playbackBlockMaxDuration = max.count();
            return *this;
        }

        ContextBuilder& setPlaybackDuration(const std::chrono::milliseconds& o) {
            playbackDuration = o.count();
            return *this;
        }

        ContextBuilder& setPlaybackSampleRate(int o) {
            playbackSampleRate = o;
            return *this;
        }

        ContextBuilder& setPlaybackCallback(Audio::QueueCallback o) {
            playbackCallback = o;
            return *this;
        }


    private:
        Analysis::PitchSolver   *pitchSolver;
        Analysis::LinpredSolver *linpredSolver;
        Analysis::FormantSolver *formantSolver;
        Analysis::InvglotSolver *invglotSolver;
        int captureSampleRate;
        int captureDuration;
        int playbackBlockMinDuration;
        int playbackBlockMaxDuration;
        int playbackDuration;
        int playbackSampleRate;
        Audio::QueueCallback playbackCallback;
    };

}

#endif // MAIN_CONTEXT_BUILDER_H
