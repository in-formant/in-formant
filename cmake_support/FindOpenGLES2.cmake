find_path(OpenGLES2_INCLUDE_DIR
    NAMES GLES2/gl2.h
)

find_library(OpenGLES2_LIBRARY
    NAMES GLESv2
    PATH_SUFFIXES lib lib64
)

set(OpenGLES2_LIBRARIES ${OpenGLES2_LIBRARY})
set(OpenGLES2_INCLUDE_DIRS ${OpenGLES2_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES2
  DEFAULT_MSG
  OpenGLES2_INCLUDE_DIR OpenGLES2_LIBRARY)

mark_as_advanced(OpenGLES2_INCLUDE_DIR OpenGLES2_LIBRARY)

