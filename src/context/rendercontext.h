#ifndef MAIN_RENDER_CONTEXT_H
#define MAIN_RENDER_CONTEXT_H

#include "../modules/renderer/renderer.h"
#include "../modules/freetype/freetype.h"
#include <memory>

using RendererBase = Module::Renderer::AbstractBase;

namespace Main {

    using namespace Module::Freetype;
    using Module::Renderer::NvgProvider;

    class RenderContextProvider : public NvgProvider {
    public:
        NVGcontext *createContext(int flags) override;
        void deleteContext(NVGcontext *ctx) override;
        void beforeBeginFrame() override;
        void afterEndFrame() override;
        void *createFramebuffer(NVGcontext *vg, int width, int height, int imageFlags) override;
        void bindFramebuffer(void *framebuffer) override;
        void deleteFramebuffer(void *framebuffer) override;
        int framebufferImage(void *framebuffer) override;
    };

    class RenderView;
    class Config;
    class DataStore;

    class FontProvider {
    public:
        virtual Font& font(int pointSize) = 0;
    };

    class RenderContext : public FontProvider {
    public:
        RenderContext(Config *config, DataStore *dataStore);
   
        void initialize();
        void terminate();
        void render();

        void setView(RenderView *view);

        void setWidth(int width);
        void setHeight(int height);
        void setDevicePixelRatio(double ratio);
        void setDPI(double horizontalDpi, double verticalDpi);

        Font& font(int pointSize) override;

    private:
        void updateSize();

        RenderContextProvider mRenderContextProvider;

        std::unique_ptr<RendererBase> mRenderer;
        std::unique_ptr<FTInstance> mFreetype;

        FontFile &mFontFile;
        Config *mConfig;
        DataStore *mDataStore;
       
        RenderView *mSelectedView;

        int mWidth;
        int mHeight;
        double mDevicePixelRatio;
        double mHorizontalDpi;
        double mVerticalDpi;

        class FontProvider {
        };
    };

    class RenderView {
    protected:
        virtual void render(RendererBase *renderer, FontProvider *fp, Config *config, DataStore *dataStore) {}

        friend void RenderContext::render();
    };

}

#endif // MAIN_RENDER_CONTEXT_H
