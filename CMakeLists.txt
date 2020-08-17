cmake_minimum_required(VERSION 3.5)
project(speech-analysis)

set(CMAKE_SUPPORT_DIR "${PROJECT_SOURCE_DIR}/cmake_support")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SUPPORT_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-fPIC -ftree-vectorize")
set(CMAKE_CXX_FLAGS_DEBUG           "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE         "-O2 -g0")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -g0")

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_SUPPORT_DIR}/CheckLowLevelExit.cmake)

set(SOURCES
    src/main.cpp
    src/modules/audio/base/base.cpp
    src/modules/audio/base/base.h
    src/modules/audio/buffer/buffer.cpp
    src/modules/audio/buffer/buffer.h
    src/modules/audio/queue/queue.cpp
    src/modules/audio/queue/queue.h
    src/modules/audio/resampler/resampler.cpp
    src/modules/audio/resampler/resampler.h
    src/modules/audio/audio.h
    src/modules/target/base/base.cpp
    src/modules/target/base/base.h
    src/modules/target/target.h
    src/modules/renderer/base/base.cpp
    src/modules/renderer/base/base.h
    src/modules/renderer/renderer.h
    src/modules/modules.h
    src/nodes/node/nodeio_audiotime.cpp
    src/nodes/node/nodeio_audiotime.h
    src/nodes/node/nodeio_audiospec.cpp
    src/nodes/node/nodeio_audiospec.h
    src/nodes/node/nodeio.cpp
    src/nodes/node/nodeio.h
    src/nodes/node/node.cpp
    src/nodes/node/node.h
    src/nodes/prereqs/prereqs.cpp
    src/nodes/prereqs/prereqs.h
    src/nodes/resampler/resampler.cpp
    src/nodes/resampler/resampler.h
    src/nodes/spectrum/spectrum.cpp
    src/nodes/spectrum/spectrum.h
    src/nodes/nodes.h
    src/analysis/fft/realfft.cpp
    src/analysis/fft/fft.h
    src/analysis/analysis.h
)

set(GL_SHADER_SOURCES
    src/modules/renderer/opengl/shaders/shader.frag
    src/modules/renderer/opengl/shaders/shader.vert
)

set(VK_SHADER_SOURCES
    src/modules/renderer/vulkan/shaders/shader.frag
    src/modules/renderer/vulkan/shaders/shader.vert
)

### REQUIRED MODULES

find_package(PkgConfig REQUIRED)
pkg_check_modules(Eigen3 REQUIRED eigen3)
pkg_check_modules(FFTW REQUIRED fftw3)

check_low_level_exit(
    LL_EXIT_FUNC
    INCLUDES unistd.h stdlib.h process.h
    NAMES _Exit _exit
)

include(ExternalProject)
ExternalProject_Add(
    soxr
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/soxr
    CMAKE_COMMAND $ENV{cross}cmake
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/soxr
        -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DWITH_OPENMP=OFF -DWITH_LSR_BINDINGS=OFF
        -DWITH_VR32=OFF -DWITH_CR32=ON -DWITH_CR64=OFF -DWITH_CR32S=ON
        -DWITH_PFFFT=OFF -DWITH_AVFFT=OFF
        -DWITH_DEV_TRACE=OFF -Wno-dev
)
set(SOXR_INCLUDE_DIR ${CMAKE_BINARY_DIR}/soxr/include)
set(SOXR_LIBRARIES -L${CMAKE_BINARY_DIR}/soxr/lib -lsoxr)

### SEARCH AUDIO MODULES 

if(TRUE)
    message(STATUS "Including audio module: dummy")
    set(AUDIO_USE_DUMMY TRUE)
    list(APPEND SOURCES
        src/modules/audio/dummy/dummy.cpp
        src/modules/audio/dummy/dummy
    )
endif()

pkg_check_modules(alsa QUIET alsa)
if(alsa_FOUND)
    message(STATUS "Including audio module: alsa")
    set(AUDIO_USE_ALSA TRUE)
    list(APPEND SOURCES
        src/modules/audio/alsa/alsa.cpp
        src/modules/audio/alsa/alsa.h
    )
endif()

pkg_check_modules(pulse QUIET libpulse)
if(pulse_FOUND)
    message(STATUS "Including audio module: pulse")
    set(AUDIO_USE_PULSE TRUE)
    list(APPEND SOURCES
        src/modules/audio/pulse/pulse.cpp
        src/modules/audio/pulse/pulse.h
    )
endif()

pkg_check_modules(portaudio QUIET portaudio-2.0)
if(portaudio_FOUND)
    message(STATUS "Including audio module: portaudio")
    set(AUDIO_USE_PORTAUDIO TRUE)
    list(APPEND SOURCES
        src/modules/audio/portaudio/portaudio.cpp
        src/modules/audio/portaudio/portaudio.h
    )
endif()

if(ANDROID)
    message(STATUS "Including audio module: oboe")
    set(OBOE_DIR ${PROJECT_SOURCE_DIR}/external/oboe)
    add_subdirectory(${OBOE_DIR} ${PROJECT_BINARY_DIR}/oboe)
    set(AUDIO_USE_OBOE TRUE)
    list(APPEND SOURCES
        src/modules/audio/oboe/oboe.cpp
        src/modules/audio/oboe/oboe.h
    )
endif()

### SEARCH TARGET MODULES

pkg_check_modules(sdl2 QUIET sdl2)
if(sdl2_FOUND)
    message(STATUS "Including target module: sdl2")
    set(TARGET_USE_SDL2 TRUE)
    list(APPEND SOURCES
        src/modules/target/sdl2/sdl2.cpp
        src/modules/target/sdl2/sdl2.h
    )
endif()

### SEARCH RENDERER MODULES 

if(TRUE)
    message(STATUS "Including renderer module: opengl")
    set(RENDERER_USE_OPENGL TRUE)
    list(APPEND SOURCES
        src/modules/renderer/opengl/opengl.cpp
        src/modules/renderer/opengl/opengl.h
    )
    if(TARGET_USE_SDL2)
        list(APPEND SOURCES
            src/modules/target/sdl2/sdl2_opengl.cpp
        )
    endif()
    include(${CMAKE_SUPPORT_DIR}/ShaderTarget.cmake)
endif()

find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    message(STATUS "Including renderer module: vulkan")
    set(RENDERER_USE_VULKAN TRUE)
    list(APPEND SOURCES
        src/modules/renderer/vulkan/vulkan.cpp
        src/modules/renderer/vulkan/vulkan.h
    )
    if(TARGET_USE_SDL2)
        list(APPEND SOURCES
            src/modules/target/sdl2/sdl2_vulkan.cpp
        )
    endif()
    include(${CMAKE_SUPPORT_DIR}/ShaderTarget.cmake)
endif()

### CREATE CMAKE TARGET

if(ANDROID)
    add_library(speech-analysis SHARED ${SOURCES})
else()
    add_executable(speech-analysis ${SOURCES})
endif()

add_dependencies(speech-analysis soxr)

target_include_directories(speech-analysis SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS} ${Eigen3_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} ${SOXR_INCLUDE_DIR})
target_link_libraries(speech-analysis PUBLIC ${SDL2_LDFLAGS} ${Eigen3_LDFLAGS} ${FFTW_LDFLAGS} ${SOXR_LIBRARIES})

if(LL_EXIT_FUNC)
    target_compile_definitions(speech-analysis PUBLIC -D__sa_llexit=${LL_EXIT_FUNC})
endif()

if(RENDERER_USE_OPENGL OR RENDERER_USE_VULKAN)
    find_file(GLM_INCLUDE_DIR REQUIRED NAMES "glm/glm.hpp")
    if(RENDERER_USE_OPENGL)
        add_shader_dependencies(TARGET speech-analysis SEMANTICS OpenGL SOURCES ${GL_SHADER_SOURCES})
    endif()
    if(RENDERER_USE_VULKAN)
        add_shader_dependencies(TARGET speech-analysis SEMANTICS Vulkan SOURCES ${VK_SHADER_SOURCES})
    endif()
endif()

if(AUDIO_USE_DUMMY)
    target_compile_definitions(speech-analysis PUBLIC -DAUDIO_USE_DUMMY=1)
endif()

if(AUDIO_USE_ALSA)
    target_include_directories(speech-analysis SYSTEM PUBLIC ${alsa_INCLUDE_DIRS})
    target_link_libraries(speech-analysis PUBLIC ${alsa_LDFLAGS})
    target_compile_definitions(speech-analysis PUBLIC -DAUDIO_USE_ALSA=1)
endif()

if(AUDIO_USE_PULSE)
    target_include_directories(speech-analysis SYSTEM PUBLIC ${pulse_INCLUDE_DIRS})
    target_link_libraries(speech-analysis PUBLIC ${pulse_LDFLAGS})
    target_compile_definitions(speech-analysis PUBLIC -DAUDIO_USE_PULSE=1)
endif()

if(AUDIO_USE_PORTAUDIO)
    target_include_directories(speech-analysis SYSTEM PUBLIC ${portaudio_INCLUDE_DIRS})
    target_link_libraries(speech-analysis PUBLIC ${portaudio_LDFLAGS})
    target_compile_definitions(speech-analysis PUBLIC -DAUDIO_USE_PORTAUDIO=1)
endif()

if(AUDIO_USE_OBOE)
    target_include_directories(speech-analysis SYSTEM PUBLIC ${OBOE_DIR}/include)
    target_link_libraries(speech-analysis PUBLIC oboe)
    target_compile_definitions(speech-analysis PUBLIC -DAUDIO_USE_OBOE=1)
endif()

if(TARGET_USE_SDL2)
    target_include_directories(speech-analysis SYSTEM PUBLIC ${sdl2_INCLUDE_DIRS})
    target_link_libraries(speech-analysis PUBLIC ${sdl2_LDFLAGS})
    target_compile_definitions(speech-analysis PUBLIC -DTARGET_USE_SDL2=1)
endif()

if(RENDERER_USE_OPENGL)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    target_sources(speech-analysis PUBLIC $ENV{GLEW_SOURCE_FILE})
    target_include_directories(speech-analysis SYSTEM PUBLIC ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})
    target_link_libraries(speech-analysis PUBLIC ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})
    target_compile_definitions(speech-analysis PUBLIC -DRENDERER_USE_OPENGL=1 -DGLEW_STATIC -DGLEW_NO_GLU)
endif()

if(RENDERER_USE_VULKAN)
    target_include_directories(speech-analysis SYSTEM PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(speech-analysis PUBLIC ${Vulkan_LIBRARIES})
    target_compile_definitions(speech-analysis PUBLIC -DRENDERER_USE_VULKAN=1)
endif()

if(EMSCRIPTEN)
    pkg_check_modules(FT2 REQUIRED freetype2)
    
    set(EMLD_FLAGS "-s FULL_ES2=1 -s \"DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[\'$autoResumeAudioContext\']\" --preload-file ${PROJECT_SOURCE_DIR}/Montserrat.ttf@Montserrat.ttf") 

    set_target_properties(speech-analysis PROPERTIES LINK_FLAGS "${EMLD_FLAGS}")
    #target_compile_options(speech-analysis PUBLIC -g4 --source-map-base /)
    target_link_libraries(speech-analysis PUBLIC ${FT2_LDFLAGS})
endif()

if(UNIX AND NOT APPLE)
    set_target_properties(speech-analysis PROPERTIES LINK_FLAGS "-pthread -static-libstdc++")
endif()

