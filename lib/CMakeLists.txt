cmake_minimum_required(VERSION 3.0)
project(libspeech)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-static")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -ftree-vectorize -s -g")

set(SOURCES
         Formant/Formant.cpp
         Formant/Formant.h
         LPC/Frame/LPC_Frame.cpp
         LPC/Frame/LPC_Frame.h
         LPC/Frame/LPC_Frame_auto.cpp
         LPC/Frame/LPC_Frame_burg.cpp
         LPC/Frame/LPC_Frame_covar.cpp
         LPC/Frame/LPC_Frame_huber.cpp
         LPC/LPC.cpp
         LPC/LPC.h
         LPC/LPC_filter.cpp
         LPC/LPC_huber.cpp
         LPC/LPC_huber.h
         LPC/LPC_huber_stat.cpp
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
         Pitch/Pitch_AMDF.cpp
         Pitch/Pitch_MPM.cpp
         Pitch/Pitch_DynWav.cpp
         Pitch/Pitch.h
         Signal/Filter.cpp
         Signal/Filter.h
         Signal/Resample.cpp
         Signal/Resample.h
         Signal/Window.cpp
         Signal/Window.h
         FFT/FFT.cpp
         FFT/FFT.h
         MFCC/MFCC.cpp
         MFCC/MFCC.h)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

include_directories(
        ${EIGEN_INCLUDE_DIR}
        ${FFTW3_INCLUDE_DIR}
)

add_library(speech ${SOURCES})

target_link_libraries(speech
        Eigen3::Eigen
        ${FFTW3_LIBRARY})

