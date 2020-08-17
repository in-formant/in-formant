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

    class AbstractBase {
    public:
        AbstractBase(std::initializer_list<Type> supportedRenderers);
        virtual ~AbstractBase();

        virtual void initialize() = 0;
        virtual void terminate() = 0;

        virtual void setTitle(const std::string& title) = 0;
        virtual void setSize(int width, int height) = 0;

        virtual void getSizeForRenderer(int *pWidth, int *pHeight) = 0;

        virtual void create() = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        virtual void close() = 0;

        virtual void processEvents() = 0;
        virtual bool shouldQuit() = 0;
        virtual bool sizeChanged() = 0;
    
        bool supportsRenderer(Type rendererType);

    public:
        OpenGLProvider *getOpenGLProvider();
        VulkanProvider *getVulkanProvider();

    protected:
        void setOpenGLProvider(OpenGLProvider *provider);
        void setVulkanProvider(VulkanProvider *provider);

    private:
        std::vector<Type> mSupportedRenderers;
        
        OpenGLProvider *mOpenGLProvider;
        VulkanProvider *mVulkanProvider;
    };

}

#endif // TARGET_BASE_H
