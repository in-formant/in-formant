#ifndef TARGET_BASE_H
#define TARGET_BASE_H

#include "../../renderer/base/base.h"
#include <initializer_list>
#include <string>
#include <vector>

namespace Module::Target {

    using Module::Renderer::Type;
    using Module::Renderer::OpenGLProvider;
    using Module::Renderer::VulkanProvider;
    using Module::Renderer::SDL2Provider;
    using Module::Renderer::NvgProvider;

    class AbstractBase {
    public:
        AbstractBase(std::initializer_list<Type> supportedRenderers);
        virtual ~AbstractBase();

        virtual void initialize() = 0;
        virtual void terminate() = 0;

        virtual void setTitle(const std::string& title) = 0;
        virtual void setSize(int width, int height) = 0;

        virtual void getSize(int *pWidth, int *pHeight) = 0;
        virtual void getSizeForRenderer(int *pWidth, int *pHeight) = 0;
        virtual void getDisplayDPI(float *hdpi, float *vdpi, float *ddpi) = 0;

        virtual void create() = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        virtual void close() = 0;
        virtual bool isVisible() const = 0;

        virtual void processEvents() = 0;
        virtual bool shouldQuit() = 0;
        virtual bool shouldClose() = 0;
        virtual bool sizeChanged() = 0;
        virtual bool isKeyPressed(uint32_t key) = 0;
        virtual bool isKeyPressedOnce(uint32_t key) = 0;
        virtual bool isMousePressed(uint32_t button) = 0;
        virtual bool isMousePressedOnce(uint32_t button) = 0;
        virtual std::pair<int, int> getMousePosition() = 0;
        virtual std::pair<float, float> getSwipeMovement() = 0;
        virtual bool isTouchPressed() = 0;

        bool supportsRenderer(Type rendererType);

    public:
        OpenGLProvider *getOpenGLProvider();
        VulkanProvider *getVulkanProvider();
        SDL2Provider *getSDL2Provider();
        NvgProvider *getNvgProvider();

    protected:
        void setOpenGLProvider(OpenGLProvider *provider);
        void setVulkanProvider(VulkanProvider *provider);
        void setSDL2Provider(SDL2Provider *provider);
        void setNvgProvider(NvgProvider *provider);

    private:
        std::vector<Type> mSupportedRenderers;
        
        OpenGLProvider *mOpenGLProvider;
        VulkanProvider *mVulkanProvider;
        SDL2Provider *mSDL2Provider;
        NvgProvider *mNvgProvider;
    };

}

#endif // TARGET_BASE_H
