#ifndef RENDERER_BASE_H
#define RENDERER_BASE_H

#include "../../freetype/freetype.h"
#include "../../../analysis/formant/formant.h"

#include <nanosvg.h>
#include <nanosvgrast.h>

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
        NanoVG,
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
        double x;
        double y;
    };
    using GraphRenderData = std::vector<GraphRenderDataPoint>;

    struct SpectrogramRenderDataPoint {
        double frequency;
        double intensity;
    };
    using SpectrogramRenderData = std::vector<SpectrogramRenderDataPoint>;

    using FrequencyTrackRenderData = std::vector<std::optional<double>>;

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

        virtual void renderGraph(const GraphRenderData& data, double pmin, double pmax, double thick, double r, double g, double b) = 0;
       
        virtual void scrollSpectrogram(const SpectrogramRenderData& data, int count) = 0;

        virtual void renderSpectrogram() = 0;

        virtual void renderFrequencyTrack(const FrequencyTrackRenderData& data, double thick, double r, double g, double b) = 0;

        virtual void renderFormantTrack(const FormantTrackRenderData& data, double thick, double r, double g, double b) = 0;

        virtual void renderFrequencyScaleBar(Module::Freetype::Font& majorFont, Module::Freetype::Font& minorFont) = 0;

        virtual double renderFrequencyCursor(double mx, double my) = 0;
        
        virtual int renderFrameCursor(double mx, double my, int count) = 0;

        virtual void renderRoundedRect(double x, double y, double w, double h, double r, double g, double b, double a) = 0;

        virtual void renderSVG(const std::string& path, double dpi, double x, double y, double w, double h) = 0;

        virtual void renderText(Module::Freetype::Font& font, const std::string& text, int x, int y, double r, double g, double b) = 0;

        virtual std::tuple<double, double, double, double> renderInputBox(Module::Freetype::Font& font, const std::string& content, int x, int y, int w, bool isFocused) = 0;

        virtual uintptr_t getContextNumber() = 0;

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

        double frequencyToCoordinate(double frequency) const;
        double coordinateToFrequency(double y) const;
        void gainToColor(double gain, double *r, double *g, double *b) const;

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

#include "parameters.h"

template<typename Tx, typename Tlo, typename Thi>
Tx clamp(const Tx& val, const Tlo& lo, const Thi& hi) {
    return Tx(val < lo ? lo : (val > hi ? hi : val));
}


#endif // RENDERER_BASE_H
