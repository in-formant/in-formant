#ifndef TARGET_SDL2_H
#define TARGET_SDL2_H

#include "../base/base.h"
#include <SDL2/SDL.h>

#ifdef RENDERER_USE_VULKAN
#   include <SDL2/SDL_vulkan.h>
#endif

namespace Module::Target {

    class SDL2 : public AbstractBase {
    public:
        SDL2(Type rendererType);
        virtual ~SDL2();

        void initialize() override;
        void terminate() override;

        void setTitle(const std::string& title) override;
        void setSize(int width, int height) override;

        void getSizeForRenderer(int *pWidth, int *pHeight) override;

        void create() override;
        void show() override;
        void hide() override;
        void close() override;

        void processEvents() override;
        bool shouldQuit() override;
        bool sizeChanged() override;

    private:
        Type mRendererType;

        std::string mTitle;
        int mWidth;
        int mHeight;

        SDL_Window *mWindow;

        bool mGotQuitEvent;
        bool mWindowSizeChanged;

        int err;
        void checkError(bool cond);
    };

#ifdef RENDERER_USE_OPENGL
    class SDL2_OpenGL : public OpenGLProvider {
    public:
        SDL2_OpenGL(SDL_Window **ptrWindow);

        virtual void createContext();
        virtual void deleteContext();

        virtual void makeCurrent();
        virtual void swapTarget();

    private:
        SDL_Window **mPtrWindow;
        SDL_GLContext mGlContext;
    };
#endif

#ifdef RENDERER_USE_VULKAN
    class SDL2_Vulkan : public VulkanProvider {
    public:
        SDL2_Vulkan(SDL_Window **ptrWindow);
    
        virtual std::vector<const char*> getRequiredExtensions();
        virtual void createSurface(VkInstance instance, VkSurfaceKHR *ptrSurface);

    private:
        SDL_Window **mPtrWindow;
    };
#endif

}

#endif // VIDEOTARGET_SDL2_H
