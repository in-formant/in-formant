#include "rendercontext.h"

using namespace Main;

NVGcontext *RenderContextProvider::createContext(int flags)
{
    return nvgCreateGL3(flags);
}

void RenderContextProvider::deleteContext(NVGcontext *ctx)
{
    nvgDeleteGL3(ctx);
}

void RenderContextProvider::beforeBeginFrame()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RenderContextProvider::afterEndFrame()
{
}

void *RenderContextProvider::createFramebuffer(NVGcontext *vg, int width, int height, int imageFlags)
{
    return nvgluCreateFramebuffer(vg, width, height, imageFlags);
}

void RenderContextProvider::bindFramebuffer(void *framebuffer)
{
    nvgluBindFramebuffer((NVGLUframebuffer *) framebuffer);
}

void RenderContextProvider::deleteFramebuffer(void *framebuffer)
{
    nvgluDeleteFramebuffer((NVGLUframebuffer *) framebuffer);
}

int RenderContextProvider::framebufferImage(void *framebuffer)
{
    return ((NVGLUframebuffer *) framebuffer)->image;
}
