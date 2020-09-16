#ifndef TARGET_SDL2_H
#define TARGET_SDL2_H

#include "../base/base.h"
#include <SDL2/SDL.h>

#ifdef RENDERER_USE_VULKAN
#   include <SDL2/SDL_vulkan.h>
#endif

#include <mutex>

namespace Module::Target {

    class SDL2 : public AbstractBase {
    public:
        SDL2(Type rendererType);
        virtual ~SDL2();

        void initialize() override;
        void terminate() override;

        void setTitle(const std::string& title) override;
        void setSize(int width, int height) override;

        void getSize(int *pWidth, int *pHeight) override;
        void getSizeForRenderer(int *pWidth, int *pHeight) override;
        void getDisplayDPI(float *hdpi, float *vdpi, float *ddpi) override;

        void create() override;
        void show() override;
        void hide() override;
        void close() override;

        void processEvents() override;
        bool shouldQuit() override;
        bool shouldClose() override;
        bool sizeChanged() override;
        bool isKeyPressed(uint32_t key) override;
        bool isMousePressed(uint32_t key) override;
        std::pair<int, int> getMousePosition() override;

    public:
#ifdef __ANDROID__
        static void prepareAssets(); 
#endif

        static std::vector<SDL_Event> globalEvents;

        Type mRendererType;

        std::string mTitle;
        int mWidth;
        int mHeight;

        SDL_Window *mWindow;

        bool mGotQuitEvent;
        bool mGotCloseEvent;
        bool mWindowSizeChanged;
    
        std::map<SDL_Scancode, bool> mKeyState;
    
        int mMouseX, mMouseY;
        uint32_t mMouseBitmask;

        int err;
        void checkError(bool cond);
    };

#if RENDERER_USE_OPENGL || RENDERER_USE_GLES
    class SDL2_OpenGL : public OpenGLProvider {
    public:
        SDL2_OpenGL(SDL_Window **ptrWindow);

        void createContext() override;
        void deleteContext() override;

        void makeCurrent() override;
        void swapTarget() override;

    private:
        SDL_Window **mPtrWindow;
        SDL_GLContext mGlContext;
    };
#endif

#ifdef RENDERER_USE_VULKAN
    class SDL2_Vulkan : public VulkanProvider {
    public:
        SDL2_Vulkan(SDL_Window **ptrWindow);
    
        std::vector<const char*> getRequiredExtensions() override;
        void createSurface(VkInstance instance, VkSurfaceKHR *surface) override;

    private:
        SDL_Window **mPtrWindow;
    };
#endif

#ifdef RENDERER_USE_SDL2
    class SDL2_Renderer : public SDL2Provider {
    public:
        SDL2_Renderer(SDL_Window **ptrWindow);

        SDL_Renderer *createRenderer(uint32_t flags) override;

    private:
        SDL_Window **mPtrWindow;
    };
#endif

#ifdef RENDERER_USE_NVG
    class SDL2_NanoVG : public NvgProvider {
    public:
        SDL2_NanoVG(SDL_Window **ptrWindow);

        NVGcontext *createContext(int flags) override;
        void deleteContext(NVGcontext *ctx) override;
       
        void beforeBeginFrame() override; 
        void afterEndFrame() override;
    
        void *createFramebuffer(NVGcontext *ctx, int width, int height, int imageFlags) override;
        void bindFramebuffer(void *framebuffer) override;
        void deleteFramebuffer(void *framebuffer) override;
        int framebufferImage(void *framebuffer) override;

    private:
        SDL_Window **mPtrWindow;
        
        void createRenderer();
        void destroyRenderer();
        SDL_Renderer *mRenderer;

        void createGLContext();
        void destroyGLContext();
        SDL_GLContext mGlContext;
    };
#endif

}

#endif // VIDEOTARGET_SDL2_H
