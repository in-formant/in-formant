#ifndef RENDERER_BASE_H
#define RENDERER_BASE_H

#include "../../freetype/freetype.h"

#include <glm/glm.hpp>

#ifdef RENDERER_USE_VULKAN
#   include <vulkan/vulkan.h>
#endif

#include <vector>

namespace Module::Renderer {

    enum class Type {
        OpenGL,
        GLES,
        Vulkan,
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
        virtual void createSurface(VkInstance instance, VkSurfaceKHR *ptrSurface) = 0;
#endif
    };

    using Vertex = glm::vec2;
    class Parameters;
    
    class AbstractBase {
    public:
        AbstractBase(Type type);
        virtual ~AbstractBase();

        virtual void setProvider(void *provider) = 0;
        virtual void initialize() = 0;
        virtual void terminate() = 0;

        virtual void begin() = 0;
        virtual void end() = 0;

        virtual void clear() = 0;

        virtual void test() = 0;

        virtual void renderGraph(float *x, float *y, size_t count) = 0;
        
        // spectrogram: [ [ [ freq, gain ] ] ]
        // lengths: [ slice_length ]
        virtual void renderSpectrogram(float ***spectrogram, size_t *lengths, size_t count) = 0;

        virtual void renderFrequencyTrack(float *track, size_t count) = 0;

        virtual void renderText(Module::Freetype::Font& font, const std::string& text, int x, int y, float r, float g, float b) = 0;

        void setDrawableSize(int width, int height);

        Parameters *getParameters();
        constexpr Type getType() { return mType; }
    
    protected:
        void getDrawableSize(int *pWidth, int *pHeight) const;

        bool hasDrawableSizeChanged() const;
        void resetDrawableSizeChanged();

    private:
        Type mType;
 
        int mDrawableWidth;
        int mDrawableHeight;
        bool mDrawableSizeChanged;

        Parameters *mParameters;
    };
}

#include "../../target/base/base.h"
#include "parameters.h"

#endif // RENDERER_BASE_H
