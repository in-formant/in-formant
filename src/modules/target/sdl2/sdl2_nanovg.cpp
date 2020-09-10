#include "sdl2.h"

#include <stdexcept>
#include <string>

using namespace Module::Target;

SDL2_NanoVG::SDL2_NanoVG(SDL_Window **ptrWindow)
    : mPtrWindow(ptrWindow)
{
}

NVGcontext *SDL2_NanoVG::createContext(int flags)
{
#if defined(NANOVG_DX11)
    createRenderer();
    ID3D11Device *device = SDL_RenderGetD3D11Device(mRenderer);
    return nvgCreateD3D11(device, flags);
#elif defined(NANOVG_METAL)
    createRenderer();
    const CAMetalLayer *layer = SDL_RenderGetMetalLayer(mMetalRenderer);
    return nvgCreateMTL(layer, flags);
#elif defined(NANOVG_GLES2)
    createGLContext();
    createRenderer();
    return nvgCreateGLES2(flags);
#elif defined(NANOVG_GLES3)
    createGLContext();
    createRenderer();
    return nvgCreateGLES3(flags);
#elif defined(NANOVG_GL3)
    createGLContext();
    createRenderer();
    
    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        throw std::runtime_error(std::string("Target::SDL2_NanoVG] Error initializing GLEW: ") + (const char *) glewGetErrorString(err));
    }

    return nvgCreateGL3(flags);
#endif
}

void SDL2_NanoVG::deleteContext(NVGcontext *ctx)
{
#if defined(NANOVG_DX11)
    nvgDeleteD3D11(ctx);
    destroyRenderer();
#elif defined(NANOVG_METAL)
    nvgDeleteMTL(ctx);
    destroyRenderer();
#elif defined(NANOVG_GLES2)
    nvgDeleteGLES2(ctx);
    destroyRenderer();
    destroyGLContext();
#elif defined(NANOVG_GLES3)
    nvgDeleteGLES3(ctx);
    destroyRenderer();
    destroyGLContext();
#elif defined(NANOVG_GL3)
    nvgDeleteGL3(ctx);
    destroyRenderer();
    destroyGLContext();
#endif
}

void SDL2_NanoVG::createRenderer()
{
    mRenderer = SDL_CreateRenderer(
            *mPtrWindow,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void SDL2_NanoVG::destroyRenderer()
{
    SDL_DestroyRenderer(mRenderer);
}

void SDL2_NanoVG::createGLContext()
{
    mGlContext = SDL_GL_CreateContext(*mPtrWindow);
    SDL_GL_MakeCurrent(*mPtrWindow, mGlContext);
}

void SDL2_NanoVG::destroyGLContext()
{
    SDL_GL_DeleteContext(mGlContext);
}

void SDL2_NanoVG::beforeBeginFrame()
{
#if defined(NANOVG_GL)
    SDL_GL_MakeCurrent(*mPtrWindow, mGlContext);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif
    SDL_RenderPresent(mRenderer);
}

void SDL2_NanoVG::afterEndFrame()
{
#if defined(NANOVG_GL)
    SDL_GL_SwapWindow(*mPtrWindow);
#endif
}

void *SDL2_NanoVG::createFramebuffer(NVGcontext *vg, int width, int height, int imageFlags)
{
#if defined(NANOVG_DX11)
    // framebuffer functions don't exist.
#elif defined(NANOVG_METAL)
    return mnvgCreateFramebuffer(vg, width, height, imageFlags);
#elif defined(NANOVG_GL)
    return nvgluCreateFramebuffer(vg, width, height, imageFlags);
#endif
}

void SDL2_NanoVG::bindFramebuffer(void *framebuffer)
{
#if defined(NANOVG_DX11)
    // framebuffer functions don't exist.
#elif defined(NANOVG_METAL)
    mnvgBindFramebuffer((MNVGframebuffer *) framebuffer);
#elif defined(NANOVG_GL)
    nvgluBindFramebuffer((NVGLUframebuffer *) framebuffer);
#endif
}

void SDL2_NanoVG::deleteFramebuffer(void *framebuffer)
{
#if defined(NANOVG_DX11)
    // framebuffer functions don't exist.
#elif defined(NANOVG_METAL)
    mnvgDeleteFramebuffer((MNVGframebuffer *) framebuffer);
#elif defined(NANOVG_GL)
    nvgluDeleteFramebuffer((NVGLUframebuffer *) framebuffer);
#endif
}

int SDL2_NanoVG::framebufferImage(void *framebuffer)
{
#if defined(NANOVG_DX11)
    // framebuffer functions don't exist.
#elif defined(NANOVG_METAL)
    return ((MNVGframebuffer *) framebuffer)->image;
#elif defined(NANOVG_GL)
    return ((NVGLUframebuffer *) framebuffer)->image;
#endif
}
