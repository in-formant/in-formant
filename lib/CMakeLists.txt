cmake_minimum_required(VERSION 3.0)
project(libspeech)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "-fPIC -mtune=generic -Wall")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g3")

set(CMAKE_CXX_FLAGS "-fPIC -mtune=generic -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3")

if(EMSCRIPTEN)
    set(EMSCRIPTEN_ARGS
        "-s WASM=1"
        "-s USE_PTHREADS=1"
        "-pthread"
        "-Wcast-align"
        "-Wover-aligned"
        "-s WARN_UNALIGNED=1"
        "-mnontrapping-fptoint"
    )
    add_definitions(${EMSCRIPTEN_ARGS})
    link_libraries(${EMSCRIPTEN_ARGS})
endif()

set(SOURCES
    Formant/Formant.cpp
    Formant/Formant.h
    Formant/track.cpp
    Formant/EKF/EKF.h
    Formant/EKF/fb2cp.cpp
    Formant/EKF/getH_FBW.cpp
    Formant/EKF/genLPCC.cpp
    Formant/EKF/init.cpp
    Formant/EKF/step.cpp
    #DeepFormants/df.h
    #DeepFormants/torch.cpp
    #DeepFormants/features.cpp
    LPC/Frame/LPC_Frame.cpp
    LPC/Frame/LPC_Frame.h
    LPC/Frame/LPC_Frame_auto.cpp
    LPC/Frame/LPC_Frame_burg.cpp
    LPC/Frame/LPC_Frame_covar.cpp
    LPC/LPC.cpp
    LPC/LPC.h
    LPC/LPC_filter.cpp
    LPC/residual.cpp
    IAIF/IAIF.h
    IAIF/IAIF.cpp
    Math/Bairstow.cpp
    Math/Bairstow.h
    Math/Polynomial.cpp
    Math/Polynomial.h
    Math/Viterbi.cpp
    Math/Viterbi.h
    Pitch/McLeod/autocorrelation.cpp
    Pitch/McLeod/parabolic_interpolation.cpp
    Pitch/McLeod/peak_picking.cpp
    Pitch/McLeod/MPM.h
    Pitch/Yin/autocorrelation.cpp
    Pitch/Yin/parabolic_interpolation.cpp
    Pitch/Yin/difference.cpp
    Pitch/Yin/cmnd.cpp
    Pitch/Yin/threshold.cpp
    #Pitch/CREPE/crepe.h
    #Pitch/CREPE/crepe.cpp
    #Pitch/CREPE/models.h
    #Pitch/CREPE/model-tiny.cpp
    #Pitch/CREPE/model-small.cpp
    #Pitch/CREPE/model-medium.cpp
    #Pitch/CREPE/model-large.cpp
    #Pitch/CREPE/model-full.cpp
    Pitch/Pitch_AMDF.cpp
    Pitch/Pitch_MPM.cpp
    Pitch/Pitch_DynWav.cpp
    Pitch/Pitch_YIN.cpp
    #Pitch/Pitch_CREPE.cpp
    Pitch/Pitch.h
    GCOI/GCOI.h
    GCOI/findpeaks.cpp
    GCOI/SEDREAMS.cpp
    GCOI/MP.cpp
    GCOI/OQ.cpp
    GCOI/gci_sedreams/gci_sedreams.h
    GCOI/gci_sedreams/sedreams.cpp
    GCOI/gci_sedreams/zerocrossings.cpp
    GCOI/gci_sedreams/findpeaks.cpp
    GCOI/gci_sedreams/median.cpp
    Signal/Filter.cpp
    Signal/Filter.h
    Signal/Resample.cpp
    Signal/Resample.h
    Signal/Window.cpp
    Signal/Window.h
    FFT/FFT.cpp
    FFT/FFT.h
    MFCC/MFCC.h
    #rpmalloc/rpmalloc.c
    #rpmalloc/rpmalloc.h
    rpmalloc.h)

if (APPLE)
    set(ENV{OSXCROSS_PKG_CONFIG_USE_NATIVE_VARIABLES} 1)
    set(PKG_CONFIG_EXECUTABLE "$ENV{OSXCROSS}/bin/$ENV{CROSS}pkg-config")
endif()

find_package(Eigen3 REQUIRED NO_MODULE)

if (FFTW_ROOT)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
endif()

find_package(FFTW REQUIRED COMPONENTS DOUBLE_LIB)

if (APPLE)
    include_directories($ENV{OSXCROSS}/SDK/MacOSX10.13.sdk/usr/include/c++/v1)
endif()

include_directories(
    ${EIGEN_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIRS}
)

if (ANDROID)
    #add_definitions(-DENABLE_PRELOAD=1)
else ()
    #if (NOT WIN32 AND NOT EMSCRIPTEN)
    #    add_definitions(-DENABLE_PRELOAD=1)
    #    if (NOT APPLE)
    #        add_definitions(-DENABLE_OVERRIDE=1)
    #    endif()
    #endif()
endif()

add_library(speech SHARED ${SOURCES})

target_link_libraries(speech
    Eigen3::Eigen
    ${FFTW_LIBRARIES}
)

