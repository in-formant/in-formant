cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0075 NEW)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_OSX_DEPLOYMENT_TARGET $ENV{MACOSX_DEPLOYMENT_TARGET})

project(in-formant VERSION ${CUR_VERSION})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_support")
include(CheckSystemName)

if(SYSTEM_ANDROID)
    include(MakeVersionCode)

    set(ANDROID_VERSION_NAME "${CUR_VERSION}")
    set(ANDROID_VERSION_CODE "${VERSION_CODE}")

    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist-res/android")

    set(ANDROID_EXTRA_LIBS_NAMES "fftw3" "c10" "torch" "torch_cpu")

    set(ANDROID_EXTRA_LIBS "")
    foreach(libname ${ANDROID_EXTRA_LIBS_NAMES})
        list(APPEND ANDROID_EXTRA_LIBS "$ENV{INS_PREFIX}/lib/lib${libname}.so")
    endforeach()

    if($ENV{ARCH} STREQUAL "arm")
        set(OMP_DIR "arm")
    elseif($ENV{ARCH} STREQUAL "arm64")
        set(OMP_DIR "aarch64")
    elseif($ENV{ARCH} STREQUAL "x86")
        set(OMP_DIR "i386")
    elseif($ENV{ARCH} STREQUAL "x86_64")
        set(OMP_DIR "x86_64")
    endif()

    list(APPEND ANDROID_EXTRA_LIBS "$ENV{ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/lib64/clang/9.0.9/lib/linux/${OMP_DIR}/libomp.so")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_FLAGS "-fPIC -ftree-vectorize -fno-omit-frame-pointer -fno-math-errno -ffast-math -mtune=generic -fvisibility=hidden")
set(CMAKE_C_FLAGS_DEBUG           "-Og -g")
set(CMAKE_C_FLAGS_RELEASE         "-O3 -g0 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO  "-O3 -g")
set(CMAKE_C_FLAGS_MINSIZEREL      "-Os -g0 -DNDEBUG")

set(CMAKE_CXX_FLAGS "-fPIC -ftree-vectorize -fno-omit-frame-pointer -fno-math-errno -ffast-math -mtune=generic -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS_DEBUG           "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE         "-O3 -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O3 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -g0 -DNDEBUG")

set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/main.cpp
    src/timetrack.h
    src/timetrack.ipp
    src/context/timings.cpp
    src/context/timings.h
    src/context/solvermakers.cpp
    src/context/solvermakers.h
    src/context/synthwrapper.cpp
    src/context/synthwrapper.h
    src/context/config.cpp
    src/context/config.h
    src/context/contextmanager.cpp
    src/context/contextmanager.h
    src/context/audiocontext.cpp
    src/context/audiocontext.h
    src/context/rendercontext.cpp
    src/context/rendercontext.h
    src/context/guicontext.cpp
    src/context/guicontext.h
    src/context/datastore.cpp
    src/context/datastore.h
    src/context/views/views.h
    src/context/views/spectrogram.cpp
    src/gui/qpainterwrapper.cpp
    src/gui/qpainterwrapper.h
    src/gui/qpainterwrapper_static.cpp
    src/gui/qpainterwrapperbase.h
    src/gui/cmap.cpp
    src/gui/canvas.cpp
    src/gui/canvas.h
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
    src/analysis/fft/fft_n.cpp
    src/analysis/fft/fft.h
    src/analysis/freqz/sosfreqz.cpp
    src/analysis/freqz/freqz.h
    src/analysis/filterbanks/log.cpp
    src/analysis/filterbanks/mel.cpp
    src/analysis/filterbanks/erb.cpp
    src/analysis/filterbanks/filterbanks.h
    src/analysis/filter/butterworth.cpp
    src/analysis/filter/zpk2sos.cpp
    src/analysis/filter/sosfilter.cpp
    src/analysis/filter/filter.cpp
    src/analysis/filter/filter.h
    #src/analysis/pitch/amdf_m.cpp
    src/analysis/pitch/yin.cpp
    src/analysis/pitch/mpm.cpp
    src/analysis/pitch/rapt.cpp
    src/analysis/pitch/rapt.h
    src/analysis/pitch/pitch.h
    src/analysis/linpred/autocorr.cpp
    src/analysis/linpred/covar.cpp
    src/analysis/linpred/burg.cpp
    src/analysis/linpred/linpred.h
    src/analysis/formant/deepformants/df.h
    src/analysis/formant/deepformants/features.cpp
    src/analysis/formant/deepformants/torch.cpp
    src/analysis/formant/deepformants.cpp
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
    src/tomlplusplus.cpp
)

### REQUIRED MODULES

find_package(PkgConfig REQUIRED)
#pkg_check_modules(Eigen3 REQUIRED eigen3)
pkg_check_modules(FFTW REQUIRED fftw3)

find_package(Eigen3 REQUIRED NO_MODULE)

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
find_package(Torch REQUIRED)
set_target_properties(torch PROPERTIES
    MAP_IMPORTED_CONFIG_DEBUG Release
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
)

find_package(Qt5 5.15 COMPONENTS Charts Quick QuickControls2 QuickTemplates2 Qml Widgets Gui Core REQUIRED)
foreach(c Charts Quick QuickControls2 QuickTemplates2 Qml Widgets Gui Core)
    if(${Qt5${c}_FOUND})
        get_target_property(Qt5${c}_LIBRARY_FILE ${Qt5${c}_LIBRARIES} LOCATION)
        message(STATUS "Found Qt5${c}: ${Qt5${c}_LIBRARY_FILE}")
    endif()
endforeach()
find_package(Qt5QuickCompiler)
qtquick_compiler_add_resources(RESOURCES_OBJ resources/qml.qrc)
qt5_add_big_resources(RESOURCES_OBJ resources/other.qrc)
set_property(SOURCE "${RESOURCES_OBJ}" PROPERTY SKIP_AUTOMOC ON)

set(ARMADILLO_INCLUDE_DIR external/armadillo/include)
set(TOMLPP_INCLUDE_DIR external/tomlplusplus/include)
set(GABORATOR_INCLUDE_DIR external/gaborator)

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

if(SYSTEM_ANDROID)
    message(STATUS "Including audio module: oboe")
    set(BUILD_SHARED_LIBS OFF)
    set(CONFIG_BAK ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    add_subdirectory(external/oboe EXCLUDE_FROM_ALL)
    set(CMAKE_BUILD_TYPE ${CONFIG_BAK})
    set(BUILD_SHARED_LIBS ON)
    set(AUDIO_USE_OBOE TRUE)
    list(APPEND SOURCES
        src/modules/audio/oboe/oboe.cpp
        src/modules/audio/oboe/oboe.h
    )
endif()

### CREATE CMAKE TARGET

if(SYSTEM_ANDROID)
    add_library(in-formant SHARED ${SOURCES} ${RESOURCES_OBJ})
else()
    add_executable(in-formant ${SOURCES} ${RESOURCES_OBJ})
endif()

target_include_directories(in-formant SYSTEM PRIVATE ${ARMADILLO_INCLUDE_DIR} ${FFTW_INCLUDE_DIRS} ${TOMLPP_INCLUDE_DIR} ${TORCH_INCLUDE_DIRS} ${GABORATOR_INCLUDE_DIR})
target_link_libraries(in-formant PRIVATE Eigen3::Eigen ${FFTW_LDFLAGS} ${TORCH_LIBRARIES} Qt5::Charts Qt5::Quick Qt5::QuickControls2 Qt5::QuickTemplates2 Qt5::Qml Qt5::Widgets Qt5::Gui Qt5::Core)

add_subdirectory(external/rpmalloc EXCLUDE_FROM_ALL)
add_subdirectory(external/soxr EXCLUDE_FROM_ALL)
target_link_libraries(in-formant PRIVATE rpmalloc soxr)
target_include_directories(in-formant SYSTEM PRIVATE external/soxr/src)

target_compile_definitions(in-formant PRIVATE
    -DINFORMANT_VERSION=${CUR_VERSION} -DARMA_DONT_USE_WRAPPER
    -DTOML_HEADER_ONLY=0 -DEIGEN_DONT_PARALLELIZE)

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
    add_dependencies(in-formant oboe)
    target_include_directories(in-formant SYSTEM PRIVATE ${OBOE_DIR}/include)
    target_link_libraries(in-formant PRIVATE oboe)
    target_compile_definitions(in-formant PRIVATE -DAUDIO_USE_OBOE=1)
endif()

## PLATFORM SPECIFIC CODE

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set_property(SOURCE ${SOURCES} PROPERTY COMPILE_FLAGS "-flto")
endif()

if(SYSTEM_WINDOWS OR SYSTEM_DARWIN OR SYSTEM_LINUX)
    #target_compile_options(in-formant PRIVATE -msse)
endif()

if(SYSTEM_DARWIN OR SYSTEM_ANDROID)
    target_include_directories(in-formant PRIVATE external/filesystem-compat/include)
endif()

if(SYSTEM_WINDOWS)
    target_compile_definitions(in-formant PRIVATE -D_WIN32_WINNT=0x600 -D_USE_MATH_DEFINES)
endif()

if(SYSTEM_DARWIN)
    target_compile_options(in-formant PRIVATE -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(SYSTEM_LINUX)
    target_link_libraries(in-formant PRIVATE -static-libstdc++ -lstdc++fs)
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fsanitize=undefined -fsanitize=address")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fsanitize=address")
    endif()
endif()

if(SYSTEM_ANDROID)
    find_package(Qt5AndroidExtras REQUIRED)
    target_link_libraries(in-formant PRIVATE Qt5::AndroidExtras GLESv2 z omp)
    target_compile_definitions(in-formant PRIVATE -DWITHOUT_SYNTH)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fuse-ld=lld")
endif()

if(SYSTEM_DARWIN)
    target_compile_definitions(in-formant PRIVATE -DGABORATOR_USE_VDSP)
else()
    set(USE_TYPE_FLOAT ON)
    set(USE_TYPE_DOUBLE ON)
    set(USE_BENCH_FFTW OFF)
    set(USE_BENCH_GREEN OFF)
    set(USE_BENCH_KISS OFF)
    set(USE_BENCH_POCKET OFF)
    add_subdirectory(external/pffft EXCLUDE_FROM_ALL)
    target_include_directories(in-formant PRIVATE external/pffft)
    target_link_libraries(in-formant PRIVATE PFFFT FFTPACK)
    target_compile_definitions(in-formant PRIVATE -DGABORATOR_USE_PFFFT)
endif()

if(WITH_PROFILER)
    pkg_check_modules(lprof REQUIRED libprofiler)
    target_include_directories(in-formant PRIVATE ${lprof_INCLUDE_DIRS})
    target_link_libraries(in-formant PRIVATE ${lprof_LIBRARIES})
    target_compile_definitions(in-formant PRIVATE -DWITH_PROFILER)
endif()
