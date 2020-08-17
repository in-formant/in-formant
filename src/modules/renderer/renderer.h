#ifndef MODULES_RENDERER_H
#define MODULES_RENDERER_H

#include "base/base.h"

#ifdef RENDERER_USE_OPENGL
#   include "opengl/opengl.h"
#endif

#ifdef RENDERER_USE_VULKAN
#   include "vulkan/vulkan.h"
#endif

#endif // MODULES_RENDERER_H
