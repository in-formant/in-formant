
find_path(OpenGLES2_INCLUDE_DIR
    NAMES GLES2/gl2.h
)

find_path(OpenGLES3_INCLUDE_DIR
    NAMES GLES3/gl32.h
)

find_library(OpenGLES3_LIBRARY
    NAMES GLESv3
    PATH_SUFFIXES lib lib64
)

set(OpenGLES_LIBRARIES ${OpenGLES3_LIBRARY})
set(OpenGLES_INCLUDE_DIRS ${OpenGLES2_INCLUDE_DIR} ${OpenGLES3_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES
  DEFAULT_MSG
  OpenGLES2_INCLUDE_DIR OpenGLES3_INCLUDE_DIR OpenGLES3_LIBRARY)

mark_as_advanced(OpenGLES2_INCLUDE_DIR OpenGLES3_INCLUDE_DIR OpenGLES3_LIBRARY)

