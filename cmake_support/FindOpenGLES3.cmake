find_path(OpenGLES3_INCLUDE_DIR
    NAMES GLES3/gl3.h
)

find_library(OpenGLES3_LIBRARY
    NAMES GLESv3
    PATH_SUFFIXES lib lib64
)

set(OpenGLES3_LIBRARIES ${OpenGLES3_LIBRARY})
set(OpenGLES3_INCLUDE_DIRS ${OpenGLES3_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES3
  DEFAULT_MSG
  OpenGLES3_INCLUDE_DIR OpenGLES3_LIBRARY)

mark_as_advanced(OpenGLES3_INCLUDE_DIR OpenGLES3_LIBRARY)

