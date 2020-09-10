#include "base.h"
#include <algorithm>
#include <stdexcept>

using namespace Module::Target;

AbstractBase::AbstractBase(std::initializer_list<Type> supportedRenderers)
    : mSupportedRenderers(supportedRenderers)
{
}

AbstractBase::~AbstractBase()
{
    if (supportsRenderer(Type::OpenGL) || supportsRenderer(Type::GLES)) {
        delete mOpenGLProvider;
    }
    if (supportsRenderer(Type::Vulkan)) {
        delete mVulkanProvider;
    }
    if (supportsRenderer(Type::SDL2)) {
        delete mSDL2Provider;
    }
    if (supportsRenderer(Type::NanoVG)) {
        delete mNvgProvider;
    }
}

bool AbstractBase::supportsRenderer(Type rendererType)
{
#ifndef RENDERER_USE_OPENGL
    if (rendererType == Type::OpenGL) return false;
#endif
#ifndef RENDERER_USE_GLES
    if (rendererType == Type::GLES) return false;
#endif
#ifndef RENDERER_USE_VULKAN
    if (rendererType == Type::Vulkan) return false;
#endif
#ifndef RENDERER_USE_SDL2
    if (rendererType == Type::SDL2) return false;
#endif
#ifndef RENDERER_USE_NVG
    if (rendererType == Type::NanoVG) return false;
#endif

    auto it = std::find(mSupportedRenderers.begin(), mSupportedRenderers.end(), rendererType);
    return (it != mSupportedRenderers.end());
}

OpenGLProvider *AbstractBase::getOpenGLProvider()
{
    if (!supportsRenderer(Type::OpenGL) && !supportsRenderer(Type::GLES)) {
        throw std::runtime_error("Target::AbstractBase] OpenGL renderer not supported");
    }
    return mOpenGLProvider;
}

void AbstractBase::setOpenGLProvider(OpenGLProvider *provider)
{
    if (!supportsRenderer(Type::OpenGL) && !supportsRenderer(Type::GLES)) {
        throw std::runtime_error("Target::AbstractBase] OpenGL renderer not supported");
    }
    mOpenGLProvider = provider;
}

VulkanProvider *AbstractBase::getVulkanProvider()
{
    if (!supportsRenderer(Type::Vulkan)) {
        throw std::runtime_error("Target::AbstractBase] Vulkan renderer not supported");
    }
    return mVulkanProvider;
}

void AbstractBase::setVulkanProvider(VulkanProvider *provider)
{
    if (!supportsRenderer(Type::Vulkan)) {
        throw std::runtime_error("Target::AbstractBase] Vulkan renderer not supported");
    }
    mVulkanProvider = provider;
}

SDL2Provider *AbstractBase::getSDL2Provider()
{
    if (!supportsRenderer(Type::SDL2)) {
        throw std::runtime_error("Target::AbstractBase] SDL2 renderer not supported");
    }
    return mSDL2Provider;
}

void AbstractBase::setSDL2Provider(SDL2Provider *provider)
{
    if (!supportsRenderer(Type::SDL2)) {
        throw std::runtime_error("Target::AbstractBase] SDL2 renderer not supported");
    }
    mSDL2Provider = provider;
}

NvgProvider *AbstractBase::getNvgProvider()
{
    if (!supportsRenderer(Type::NanoVG)) {
        throw std::runtime_error("Target::AbstractBase] NanoVG renderer not supported");
    }
    return mNvgProvider;
}

void AbstractBase::setNvgProvider(NvgProvider *provider)
{
    if (!supportsRenderer(Type::NanoVG)) {
        throw std::runtime_error("Target::AbstractBase] NanoVG renderer not supported");
    }
    mNvgProvider = provider;
}
