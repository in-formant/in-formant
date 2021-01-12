#ifndef RENDERER_NVG_GL_H
#define RENDERER_NVG_GL_H

#if ! ( /* defined(_WIN32) &&  defined(__APPLE__) */ 0 )
#  if defined(__EMSCRIPTEN__) 
#    include <GLES2/gl2.h>
#    define NANOVG_GLES2
#  elif defined(ANDROID) || defined(__ANDROID__)
#    include <GLES3/gl3.h>
#    define NANOVG_GLES3
#  else
#    include <GL/gl.h>
#    define NANOVG_GL3
#  endif
#  include <nanovg_gl.h>
extern "C" {
#  include <nanovg_gl_utils.h>
}
#  define NANOVG_GL
#endif

#endif
