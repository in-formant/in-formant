#include "qt5quick.h"

#include <stdexcept>
#include <string>

using namespace Module::Target;

Q5Q_NanoVG::Q5Q_NanoVG()
{
}

NVGcontext *Q5Q_NanoVG::createContext(int flags)
{
#ifdef NANOVG_GL3
    createGLContext();
    return nvgCreateGL3(flags);
#endif
}

void Q5Q_NanoVG::deleteContext(NVGcontext *ctx)
{
#ifdef NANOVG_GL3
    nvgDeleteGL3(ctx);
    destroyGLContext();
#endif
}

void Q5Q_NanoVG::createGLContext()
{
}

void Q5Q_NanoVG::destroyGLContext()
{
}

void Q5Q_NanoVG::beforeBeginFrame()
{
#if defined(NANOVG_GL)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif
}

void Q5Q_NanoVG::afterEndFrame()
{
#if defined(NANOVG_GL)
    //SDL_GL_SwapWindow(*mPtrWindow);
#endif
}

void *Q5Q_NanoVG::createFramebuffer(NVGcontext *vg, int width, int height, int imageFlags)
{
#if defined(NANOVG_GL)
    return nvgluCreateFramebuffer(vg, width, height, imageFlags);
#endif
}

void Q5Q_NanoVG::bindFramebuffer(void *framebuffer)
{
#if defined(NANOVG_GL)
    nvgluBindFramebuffer((NVGLUframebuffer *) framebuffer);
#endif
}

void Q5Q_NanoVG::deleteFramebuffer(void *framebuffer)
{
#if defined(NANOVG_GL)
    nvgluDeleteFramebuffer((NVGLUframebuffer *) framebuffer);
#endif
}

int Q5Q_NanoVG::framebufferImage(void *framebuffer)
{
#if defined(NANOVG_GL)
    return ((NVGLUframebuffer *) framebuffer)->image;
#endif
}
