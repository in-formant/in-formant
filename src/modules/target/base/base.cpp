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
