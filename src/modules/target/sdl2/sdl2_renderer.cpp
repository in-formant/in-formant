#include "sdl2.h"

using namespace Module::Target;

SDL2_Renderer::SDL2_Renderer(SDL_Window **ptrWindow)
    : mPtrWindow(ptrWindow)
{
}

SDL_Renderer *SDL2_Renderer::createRenderer(uint32_t flags)
{
    return SDL_CreateRenderer(*mPtrWindow, -1, flags);
}
