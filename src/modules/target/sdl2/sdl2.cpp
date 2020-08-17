#include "sdl2.h"
#include <stdexcept>

using namespace Module::Target;

SDL2::SDL2(Type rendererType)
    : AbstractBase { Type::OpenGL, Type::Vulkan },
      mRendererType(rendererType),
      mWindow(nullptr)
{
#ifdef RENDERER_USE_OPENGL
    setOpenGLProvider(new SDL2_OpenGL(&mWindow));
#endif

#ifdef RENDERER_USE_VULKAN
    setVulkanProvider(new SDL2_Vulkan(&mWindow));
#endif
}

SDL2::~SDL2()
{
}

void SDL2::initialize()
{
    err = SDL_Init(SDL_INIT_VIDEO);
    checkError(err < 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

void SDL2::terminate()
{
    SDL_Quit();
}

void SDL2::setTitle(const std::string& title)
{
    mTitle = title;

    if (mWindow != nullptr) {
        SDL_SetWindowTitle(mWindow, title.c_str());
    }
}

void SDL2::setSize(int width, int height)
{
    mWidth = width;
    mHeight = height;

    if (mWindow != nullptr) {
        SDL_SetWindowSize(mWindow, width, height);
    }
}

void SDL2::getSizeForRenderer(int *pWidth, int *pHeight)
{
    if (mRendererType == Type::OpenGL) {
        SDL_GL_GetDrawableSize(mWindow, pWidth, pHeight);
    }
    else if (mRendererType == Type::Vulkan) {
#ifdef RENDERER_USE_VULKAN
        SDL_Vulkan_GetDrawableSize(mWindow, pWidth, pHeight);
#endif
    }
    else {
        SDL_GetWindowSize(mWindow, pWidth, pHeight);
    }
}

void SDL2::create()
{
    SDL_WindowFlags backendFlag;

    switch (mRendererType) {
    case Type::OpenGL:
        backendFlag = SDL_WINDOW_OPENGL; 
        break;
    case Type::Vulkan:
        backendFlag = SDL_WINDOW_VULKAN;
        break;
    }

    mWindow = SDL_CreateWindow(
            mTitle.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            mWidth,
            mHeight,
            backendFlag | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    checkError(mWindow == nullptr);

    mGotQuitEvent = false;
    mWindowSizeChanged = false;
}

void SDL2::show()
{
    SDL_ShowWindow(mWindow);
    SDL_RaiseWindow(mWindow);
}

void SDL2::hide()
{
    SDL_HideWindow(mWindow);
}

void SDL2::close()
{
    SDL_DestroyWindow(mWindow);
}

void SDL2::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            mGotQuitEvent = true;
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                mGotQuitEvent = true;
            }
        }
        else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                mWindowSizeChanged = true;
            }
        }
    }
}

bool SDL2::shouldQuit()
{
    return mGotQuitEvent;
}

bool SDL2::sizeChanged()
{
    if (mWindowSizeChanged) {
        mWindowSizeChanged = false;
        return true;
    }
    return false;
}

void SDL2::checkError(bool cond)
{
    if (cond) {
        throw std::runtime_error(std::string("Target::SDL2] ") + SDL_GetError());
    }
}

