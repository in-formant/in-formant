#ifndef MODULES_RENDERER_H
#define MODULES_RENDERER_H

#include "base/base.h"

#ifdef RENDERER_USE_OPENGL
#   include "opengl/opengl.h"
#endif

#ifdef RENDERER_USE_GLES
#   include "gles/gles.h"
#endif

#ifdef RENDERER_USE_VULKAN
#   include "vulkan/vulkan.h"
#endif

#ifdef RENDERER_USE_SDL2
#   include "sdl2/sdl2.h"
#endif

#ifdef RENDERER_USE_NVG
#   include "nanovg/nanovg.h"
#endif

#endif // MODULES_RENDERER_H
