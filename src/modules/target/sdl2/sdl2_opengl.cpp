#include "sdl2.h"

using namespace Module::Target;

SDL2_OpenGL::SDL2_OpenGL(SDL_Window **ptrWindow)
    : mPtrWindow(ptrWindow)
{
}

void SDL2_OpenGL::createContext()
{
    mGlContext = SDL_GL_CreateContext(*mPtrWindow);
}

void SDL2_OpenGL::deleteContext()
{
    SDL_GL_DeleteContext(mGlContext);
}

void SDL2_OpenGL::makeCurrent()
{
    SDL_GL_MakeCurrent(*mPtrWindow, mGlContext);
}

void SDL2_OpenGL::swapTarget()
{
    SDL_GL_SwapWindow(*mPtrWindow);
}

