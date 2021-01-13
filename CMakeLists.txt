cmake_minimum_required(VERSION 3.5)
set(CMAKE_OSX_DEPLOYMENT_TARGET $ENV{MACOSX_DEPLOYMENT_TARGET})
project(in-formant)

set(CMAKE_SUPPORT_DIR "${PROJECT_SOURCE_DIR}/cmake_support")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SUPPORT_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-fPIC -ftree-vectorize -fno-omit-frame-pointer -fno-math-errno -mtune=generic")
set(CMAKE_CXX_FLAGS_DEBUG           "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE         "-O2 -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -g0 -DNDEBUG")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "-fuse-ld=gold")

if(ANDROID)
elseif(WIN32 OR APPLE OR UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i686 -msse")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -msse")
    endif()
endif()

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/main.cpp
    src/context/context.h
    src/context/contextbuilder.h
    src/context/contextmanager.cpp
    src/context/ctxmgr_spectrogram.cpp
    src/context/ctxmgr_fftspectrum.cpp
    src/context/ctxmgr_oscilloscope.cpp
    src/context/ctxmgr_settings.cpp
    src/context/ctxmgr_synth.cpp
    src/context/contextmanager.h
    src/gui/gui.h
    src/gui/canvas.h
    src/gui/canvas.cpp
    src/gui/app.cpp
    src/backtrace/backtrace.h
    src/android_redirect_log.cpp
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
    src/modules/renderer/base/parameters.h
    src/modules/renderer/base/base.h
    src/modules/renderer/renderer.h
    src/modules/app/pipeline/pipeline.cpp
    src/modules/app/pipeline/pipeline.h
    src/modules/app/synthesizer/synthesizer.cpp
    src/modules/app/synthesizer/synthesizer.h
    src/modules/app/app.h
    src/modules/modules.h
    src/nodes/node/nodeio_audiotime.cpp
    src/nodes/node/nodeio_audiotime.h
    src/nodes/node/nodeio_audiospec.cpp
    src/nodes/node/nodeio_audiospec.h
    src/nodes/node/nodeio_frequencies.cpp
    src/nodes/node/nodeio_frequencies.h
    src/nodes/node/nodeio_iirfilter.cpp
    src/nodes/node/nodeio_iirfilter.h
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
    src/nodes/tail/tail.cpp
    src/nodes/tail/tail.h
    src/nodes/pitchtracker/pitchtracker.cpp
    src/nodes/pitchtracker/pitchtracker.h
    src/nodes/preemph/preemph.cpp
    src/nodes/preemph/preemph.h
    src/nodes/linpred/linpred.cpp
    src/nodes/linpred/linpred.h
    src/nodes/formants/formants.cpp
    src/nodes/formants/formants.h
    src/nodes/invglot/invglot.cpp
    src/nodes/invglot/invglot.h
    src/nodes/nodes.h
    src/analysis/fft/realfft.cpp
    src/analysis/fft/complexfft.cpp
    src/analysis/fft/realrealfft.cpp
    src/analysis/fft/wisdom.cpp
    src/analysis/fft/fft.h
    src/analysis/filter/butterworth.cpp
    src/analysis/filter/zpk2sos.cpp
    src/analysis/filter/sosfilter.cpp
    src/analysis/filter/filter.cpp
    src/analysis/filter/filter.h
    src/analysis/pitch/amdf_m.cpp
    src/analysis/pitch/yin.cpp
    src/analysis/pitch/mpm.cpp
    src/analysis/pitch/rapt.cpp
    src/analysis/pitch/rapt.h
    src/analysis/pitch/pitch.h
    src/analysis/linpred/autocorr.cpp
    src/analysis/linpred/burg.cpp
    src/analysis/linpred/linpred.h
    src/analysis/formant/simplelp.cpp
    src/analysis/formant/filteredlp.cpp
    src/analysis/formant/formant.h
    src/analysis/invglot/iaif.cpp
    src/analysis/invglot/gfm_iaif.cpp
    #src/analysis/invglot/amgif.cpp
    src/analysis/invglot/invglot.h
    src/analysis/wavelet/wavelet.h
    src/analysis/wavelet/wavelets.cpp
    src/analysis/wavelet/wavelets.h
    src/analysis/wavelet/common.cpp
    src/analysis/wavelet/common.h
    src/analysis/wavelet/coeffs.cpp
    src/analysis/wavelet/coeffs.h
    src/analysis/wavelet/convolution.cpp
    src/analysis/wavelet/convolution.h
    src/analysis/wavelet/wt.cpp
    src/analysis/wavelet/wt.h
    src/analysis/gci/sigma.cpp
    src/analysis/gci/sigma.h
    src/analysis/gci/xewgrdel.cpp
    src/analysis/util/find_peaks.cpp
    src/analysis/util/find_roots.cpp
    src/analysis/util/laguerre.cpp
    src/analysis/util/laguerre.h
    src/analysis/util/polish_root.cpp
    src/analysis/util/eval_polynomial.cpp
    src/analysis/util/calc_formant.cpp
    src/analysis/util/sort_formants.cpp
    src/analysis/util/parabolic_interpolation.cpp
    src/analysis/util/zerocros.cpp
    src/analysis/util/util.h
    src/analysis/analysis.h
    src/synthesis/noise.cpp
    src/synthesis/filter.cpp
    src/synthesis/sosfilter.cpp
    src/synthesis/glotLf.cpp
    src/synthesis/shiftFilter.cpp
    src/synthesis/polyFromRoots.cpp
    src/synthesis/synthesis.h
    external/speex/speex_resampler.h
    external/speex/speex_resampler.c
)

set(RESOURCES
    resources/main.qrc
)

### REQUIRED MODULES

find_package(PkgConfig REQUIRED)
pkg_check_modules(Eigen3 REQUIRED eigen3)
pkg_check_modules(FFTW REQUIRED fftw3)

find_package(Qt5 5.12 COMPONENTS Quick QuickControls2 Qml Widgets Gui Core REQUIRED)
foreach(c Quick QuickControls2 Qml Widgets Gui Core)
    if(${Qt5${c}_FOUND})
        get_target_property(Qt5${c}_LIBRARY_FILE ${Qt5${c}_LIBRARIES} LOCATION)
        message(STATUS "Found Qt5${c}: ${Qt5${c}_LIBRARY_FILE}")
    endif()
endforeach()
find_package(Qt5QuickCompiler REQUIRED)
qtquick_compiler_add_resources(RESOURCES_OBJ ${RESOURCES})

set(SPEEX_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/external/speex)
set(NANOSVG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/external/nanosvg)
set(ARMADILLO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/external/armadillo/include)

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    add_subdirectory(${OBOE_DIR} ${PROJECT_BINARY_DIR}/oboe)
    set(AUDIO_USE_OBOE TRUE)
    list(APPEND SOURCES
        src/modules/audio/oboe/oboe.cpp
        src/modules/audio/oboe/oboe.h
    )
endif()

if(EMSCRIPTEN)
    message(STATUS "Including audio module: webaudio")
    set(AUDIO_USE_WEBAUDIO TRUE)
    list(APPEND SOURCES
        src/modules/audio/webaudio/webaudio.cpp
        src/modules/audio/webaudio/webaudio.h)
endif()

### SEARCH TARGET MODULES

message(STATUS "Including target module: qt5quick")
set(TARGET_USE_QT5QUICK TRUE)
list(APPEND SOURCES
    src/modules/target/qt5quick/qt5quick.cpp
    src/modules/target/qt5quick/qt5quick.h
)

### SEARCH RENDERER MODULES 

message(STATUS "Including renderer module: nanovg")
set(NVG_DIR ${PROJECT_SOURCE_DIR}/external/nanovg)
add_subdirectory(${NVG_DIR} ${PROJECT_BINARY_DIR}/nanovg)
set(RENDERER_USE_NVG TRUE)
list(APPEND SOURCES
    src/modules/renderer/nanovg/nanovg.cpp
    src/modules/renderer/nanovg/nanovg.h
    src/modules/renderer/nanovg/fontattachment.cpp
    src/modules/renderer/nanovg/nvg_gl.c
    src/modules/renderer/nanovg/nvg_gl.h)
if(TARGET_USE_SDL2)
    list(APPEND SOURCES
        src/modules/target/sdl2/sdl2_nanovg.cpp
    )
endif()
if(TARGET_USE_QT5QUICK)
    list(APPEND SOURCES
        src/modules/target/qt5quick/q5q_nanovg.cpp
    )
endif()

### FREETYPE MODULE

message(STATUS "Including font module: freetype")
set(FREETYPE_DIR ${PROJECT_SOURCE_DIR}/external/freetype)
set(BUILD_SHARED_LIBS TRUE)
add_subdirectory(${FREETYPE_DIR} ${PROJECT_BINARY_DIR}/freetype)
set(FONT_USE_FREETYPE TRUE)
list(APPEND SOURCES
    src/modules/freetype/instance.cpp
    src/modules/freetype/fontfile.cpp
    src/modules/freetype/font.cpp
    src/modules/freetype/freetype.h
)

### CREATE CMAKE TARGET

if(ANDROID)
    add_library(in-formant SHARED ${SOURCES} ${RESOURCES_OBJ})
else()
    add_executable(in-formant ${SOURCES} ${RESOURCES_OBJ})
endif()

target_include_directories(in-formant SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS} ${Eigen3_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} ${SPEEX_INCLUDE_DIR} ${NANOSVG_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIR})
target_link_libraries(in-formant PRIVATE ${SDL2_LDFLAGS} ${Eigen3_LDFLAGS} ${FFTW_LDFLAGS} Qt5::Quick Qt5::QuickControls2 Qt5::Qml Qt5::Widgets Qt5::Gui Qt5::Core)

target_compile_definitions(in-formant PRIVATE -DARMA_DONT_USE_WRAPPER)

if(AUDIO_USE_DUMMY)
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_DUMMY=1)
endif()

if(AUDIO_USE_ALSA)
    target_include_directories(in-formant SYSTEM PRIVATE ${alsa_INCLUDE_DIRS})
    target_link_libraries(in-formant PRIVATE ${alsa_LDFLAGS})
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_ALSA=1)
endif()

if(AUDIO_USE_PULSE)
    target_include_directories(in-formant SYSTEM PRIVATE ${pulse_INCLUDE_DIRS})
    target_link_libraries(in-formant PRIVATE PRIVATE ${pulse_LDFLAGS})
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_PULSE=1)
endif()

if(AUDIO_USE_PORTAUDIO)
    target_include_directories(in-formant SYSTEM PRIVATE ${portaudio_INCLUDE_DIRS})
    target_link_libraries(in-formant PRIVATE ${portaudio_LIBRARIES})
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_PORTAUDIO=1)
endif()

if(AUDIO_USE_OBOE)
    target_include_directories(in-formant SYSTEM PRIVATE ${OBOE_DIR}/include)
    target_link_libraries(in-formant PRIVATE oboe)
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_OBOE=1)
endif()

if(AUDIO_USE_WEBAUDIO)
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_WEBAUDIO=1)
endif()

if(TARGET_USE_SDL2)
    target_include_directories(in-formant SYSTEM PRIVATE ${sdl2_INCLUDE_DIRS})
    target_link_libraries(in-formant PRIVATE ${sdl2_LDFLAGS})
    target_compile_definitions(in-formant PRIVATE -DTARGET_USE_SDL2=1)
endif()

if(TARGET_USE_QT5QUICK)
    target_compile_definitions(in-formant PRIVATE -DTARGET_USE_QT5QUICK=1)
endif()

if(RENDERER_USE_NVG)
    target_include_directories(in-formant SYSTEM PRIVATE ${NVG_DIR}/src)
    target_link_libraries(in-formant PRIVATE nanovg)
    target_compile_definitions(in-formant PRIVATE -DRENDERER_USE_NVG=1)

#[[ Future code for Metal render support
    if(APPLE)
        find_package(Metal REQUIRED)
        target_include_directories(in-formant SYSTEM PRIVATE ${METAL_INCLUDE_DIR})
        target_link_libraries(in-formant PRIVATE PRIVATE ${METAL_LIBRARIES})
    elseif(ANDROID)
]]

    if(ANDROID)
        find_package(OpenGLES3 REQUIRED)
        target_include_directories(in-formant SYSTEM PRIVATE ${OpenGLES3_INCLUDE_DIRS})
        target_link_libraries(in-formant PRIVATE ${OpenGLES3_LIBRARIES})
    elseif(EMSCRIPTEN)
        #find_package(OpenGLES2 REQUIRED)
        #target_include_directories(in-formant SYSTEM PRIVATE ${OpenGLES2_INCLUDE_DIRS})
        #target_link_libraries(in-formant PRIVATE ${OpenGLES2_LIBRARIES})
    elseif(UNIX OR WIN32 OR APPLE)
        set(OpenGL_GL_PREFERENCE GLVND)
        find_package(OpenGL REQUIRED)
        target_include_directories(in-formant SYSTEM PRIVATE ${OPENGL_INCLUDE_DIRS})
        target_link_libraries(in-formant PRIVATE ${OPENGL_LIBRARIES})

        #target_sources(in-formant PRIVATE $ENV{GLEW_SOURCE_FILE})
        #target_compile_definitions(in-formant PRIVATE -DGLEW_STATIC -DGLEW_NO_GLU)
    endif()
endif()

if(FONT_USE_FREETYPE)
    target_include_directories(in-formant SYSTEM PRIVATE ${FREETYPE_DIR}/include)
    target_link_libraries(in-formant PRIVATE freetype)
    target_compile_definitions(in-formant PRIVATE -DFONT_USE_FREETYPE=1)
endif()

## PLATFORM SPECIFIC CODE

set(RESAMPLER_QUALITY SOXR_HQ)

if(EMSCRIPTEN)
    set(EM_FLAGS "-sENVIRONMENT=web" "-sASSERTIONS=2" "-sINITIAL_MEMORY=32MB" "-sALLOW_MEMORY_GROWTH=1" "--no-heap-copy" "-sEMULATE_FUNCTION_POINTER_CASTS=1" "-sERROR_ON_UNDEFINED_SYMBOLS=1" "-sUSE_PTHREADS=0" #[["-sFULL_ES2=1"]] "-sEXIT_RUNTIME=1")

    set(EMCC_FLAGS ${EM_FLAGS})
    set(EMLD_FLAGS ${EM_FLAGS} "-sEXPORTED_FUNCTIONS=[_main,_webaudio_process_playback,_webaudio_process_capture]" "-sEXPORTED_RUNTIME_METHODS=[ccall]" "-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[$autoResumeAudioContext]" "--preload-file ${PROJECT_SOURCE_DIR}/Montserrat.otf@Montserrat.otf")

    if (CMAKE_BUILD_TYPE MATCHES "^Debug$")
        target_compile_options(in-formant PRIVATE ${EMCC_FLAGS} -g -g4 --source-map-base /)
        target_link_libraries(in-formant PRIVATE ${EMLD_FLAGS} -g -g4 "--source-map-base /")
    elseif (CMAKE_BUILD_TYPE MATCHES "^Release$")
        target_compile_options(in-formant PRIVATE ${EMCC_FLAGS} -fno-rtti)
        target_link_libraries(in-formant PRIVATE ${EMLD_FLAGS} -fno-rtti)
    else()
        target_compile_options(in-formant PRIVATE ${EMCC_FLAGS})
        target_link_libraries(in-formant PRIVATE ${EMLD_FLAGS})
    endif()
endif()

if(ANDROID)
    target_sources(in-formant PRIVATE src/context/ctxmgr_androidcommon.cpp)
    if(TARGET_USE_SDL2)
        target_sources(in-formant PRIVATE src/modules/target/sdl2/sdl2_android.cpp)
    endif()
    target_link_libraries(in-formant PRIVATE android)
endif()

if(UNIX AND NOT EMSCRIPTEN)
    target_sources(in-formant PRIVATE src/backtrace/unix.cpp)
    target_link_libraries(in-formant PRIVATE dl)
    set_target_properties(in-formant PROPERTIES LINK_FLAGS "-pthread -static-libstdc++")
endif()

if(NOT ANDROID AND NOT WIN32 AND CMAKE_BUILD_TYPE MATCHES "^Debug$")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fsanitize=address -fsanitize=undefined")
endif()

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_sources(in-formant PRIVATE src/backtrace/windows.cpp)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_sources(in-formant PRIVATE src/backtrace/windows64.cpp)
    endif()
    target_link_libraries(in-formant PRIVATE ntdll dbghelp)
endif()

target_compile_definitions(in-formant PRIVATE -DRESAMPLER_QUALITY=${RESAMPLER_QUALITY})
