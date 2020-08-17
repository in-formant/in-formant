#include "sdl2.h"
#include <stdexcept>

using namespace Module::Target;

SDL2_Vulkan::SDL2_Vulkan(SDL_Window **ptrWindow)
    : mPtrWindow(ptrWindow)
{
}

std::vector<const char *> SDL2_Vulkan::getRequiredExtensions()
{ 
    unsigned int count;
    if (!SDL_Vulkan_GetInstanceExtensions(*mPtrWindow, &count, nullptr)) {
        throw std::runtime_error("Target::SDL2_Vulkan] Not all required Vulkan extensions were returned.");
    }

    std::vector<const char *> extensions(count);
    if (!SDL_Vulkan_GetInstanceExtensions(*mPtrWindow, &count, extensions.data())) {
        throw std::runtime_error("Target::SDL2_Vulkan] Not all required Vulkan extensions were returned.");
    }

    return extensions;
}

void SDL2_Vulkan::createSurface(VkInstance instance, VkSurfaceKHR *ptrSurface)
{
    if (!SDL_Vulkan_CreateSurface(*mPtrWindow, instance, ptrSurface)) {
        throw std::runtime_error("Target::SDL2_Vulkan] Error creating Vulkan surface");
    }
}
