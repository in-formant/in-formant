#ifndef RENDERER_BASE_H
#define RENDERER_BASE_H

#include "../../freetype/freetype.h"
#include "../../../analysis/formant/formant.h"

#ifdef RENDERER_USE_VULKAN
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#   define VULKAN_HPP_TYPESAFE_CONVERSION 1
#   include <vulkan/vulkan.hpp>
#endif

#ifdef RENDERER_USE_SDL2
#   include <SDL2/SDL.h>
#endif

#ifdef RENDERER_USE_NVG
#  include <nanovg.h>
#  if defined(__WIN32)
//#    include <nanovg_dx11.h>
//#    define NANOVG_DX11
#    include "../nanovg/nvg_gl.h"
#  elif defined(__APPLE__)
//#    include <nanovg_mtl.h>
//#    define NANOVG_METAL
#    include "../nanovg/nvg_gl.h"
#  else
#    include "../nanovg/nvg_gl.h"
#  endif
#endif

#include <vector>
#include <map>

namespace Module::Renderer {

    enum class Type {
        OpenGL,
        GLES,
        Vulkan,
        SDL2,
        NanoVG,
    };

    class OpenGLProvider {
#if RENDERER_USE_OPENGL || RENDERER_USE_GLES
    public:
        virtual ~OpenGLProvider() {}

        virtual void createContext() = 0;
        virtual void deleteContext() = 0;

        virtual void makeCurrent() = 0;
        virtual void swapTarget() = 0;
#endif
    };

    class VulkanProvider {
#ifdef RENDERER_USE_VULKAN
    public:
        virtual ~VulkanProvider() {}

        virtual std::vector<const char *> getRequiredExtensions() = 0;
        virtual void createSurface(VkInstance instance, VkSurfaceKHR *surface) = 0;
#endif
    };

    class SDL2Provider {
#ifdef RENDERER_USE_SDL2
    public:
        virtual ~SDL2Provider() {}
        
        virtual SDL_Renderer *createRenderer(uint32_t flags) = 0;
#endif
    };
    
    class NvgProvider {
#ifdef RENDERER_USE_NVG
    public:
        virtual ~NvgProvider() {}
        virtual NVGcontext *createContext(int flags) = 0;
        virtual void deleteContext(NVGcontext *ctx) = 0;
        
        virtual void beforeBeginFrame() = 0;
        virtual void afterEndFrame() = 0;

        virtual void *createFramebuffer(NVGcontext *ctx, int width, int height, int imageFlags) = 0;
        virtual void bindFramebuffer(void *framebuffer) = 0;
        virtual void deleteFramebuffer(void *framebuffer) = 0;
        virtual int framebufferImage(void *framebuffer) = 0;
#endif
    };

    class Parameters;

    struct GraphRenderDataPoint {
        float x;
        float y;
    };
    using GraphRenderData = std::vector<GraphRenderDataPoint>;

    struct SpectrogramRenderDataPoint {
        float frequency;
        float intensity;
    };
    using SpectrogramRenderData = std::vector<SpectrogramRenderDataPoint>;

    using FrequencyTrackRenderData = std::vector<std::optional<float>>;

    using FormantTrackRenderData = std::vector<std::optional<Analysis::FormantData>>;

    class AbstractBase { 
    public:
        AbstractBase(Type type);
        virtual ~AbstractBase();

        virtual void setProvider(void *provider) = 0;
        virtual void initialize() = 0;
        virtual void terminate() = 0;

        virtual void begin() = 0;
        virtual void end() = 0;

        virtual void test() = 0;

        virtual void renderGraph(const GraphRenderData& data, float thick, float r, float g, float b) = 0;
        
        virtual void renderSpectrogram(const SpectrogramRenderData& data, int count) = 0;

        virtual void renderFrequencyTrack(const FrequencyTrackRenderData& data, float thick, float r, float g, float b) = 0;

        virtual void renderFormantTrack(const FormantTrackRenderData& data, float r, float g, float b) = 0;

        virtual void renderFrequencyScaleBar(Module::Freetype::Font& majorFont, Module::Freetype::Font& minorFont) = 0;

        virtual void renderText(Module::Freetype::Font& font, const std::string& text, int x, int y, float r, float g, float b) = 0;

        virtual std::tuple<float, float, float, float> renderInputBox(Module::Freetype::Font& font, const std::string& content, int x, int y, int w, bool isFocused) = 0;

        void setDrawableSize(int width, int height);
        void setWindowSize(int width, int height);

        Parameters *getParameters();
        constexpr Type getType() { return mType; }

    protected:
        void getDrawableSize(int *pWidth, int *pHeight) const;
        void getWindowSize(int *pWidth, int *pHeight) const;

        bool hasDrawableSizeChanged() const;
        void resetDrawableSizeChanged();

        bool hasWindowSizeChanged() const;
        void resetWindowSizeChanged();

        float frequencyToCoordinate(float frequency) const;
        void gainToColor(float gain, float *r, float *g, float *b) const;

    private:
        Type mType;
 
        int mDrawableWidth;
        int mDrawableHeight;
        bool mDrawableSizeChanged;

        int mWindowWidth;
        int mWindowHeight;
        bool mWindowSizeChanged;

        Parameters *mParameters;
    };
}

#include "../../target/base/base.h"
#include "parameters.h"

template<typename Tx, typename Tlo, typename Thi>
Tx clamp(const Tx& val, const Tlo& lo, const Thi& hi) {
    return Tx(val < lo ? lo : (val > hi ? hi : val));
}


#endif // RENDERER_BASE_H
