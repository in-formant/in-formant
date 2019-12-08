cmake_minimum_required(VERSION 3.8)
project(main)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-fopenmp -fext-numeric-literals")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -ftree-vectorize -s -g")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
        main.cpp
        Exceptions.cpp
        Exceptions.h
        analysis/Analyser_mainLoop.cpp
        analysis/Analyser.cpp
        analysis/Analyser.h
        analysis/parts/formants.cpp
        analysis/parts/lpc.cpp
        analysis/parts/pitch.cpp
        analysis/parts/preemph.cpp
        analysis/parts/resample.cpp
        analysis/parts/spectrum.cpp
        audio/AudioCapture.cpp
        audio/AudioCapture_callbacks.cpp
        audio/AudioCapture.h
        audio/RingBuffer.cpp
        audio/RingBuffer.h
        gui/canvas/AnalyserCanvas.cpp
        gui/canvas/AnalyserCanvas.h
        gui/qt/MainWindow.cpp
        gui/qt/MainWindow.h
        lib/Formant/Formant.cpp
        lib/Formant/Formant.h
        lib/Formant/DF/features.cpp
        lib/Formant/DF/features.h
        lib/LPC/Frame/LPC_Frame.cpp
        lib/LPC/Frame/LPC_Frame.h
        lib/LPC/Frame/LPC_Frame_auto.cpp
        lib/LPC/Frame/LPC_Frame_burg.cpp
        lib/LPC/Frame/LPC_Frame_covar.cpp
        lib/LPC/Frame/LPC_Frame_huber.cpp
        lib/LPC/LPC.cpp
        lib/LPC/LPC.h
        lib/LPC/LPC_filter.cpp
        lib/LPC/LPC_huber.cpp
        lib/LPC/LPC_huber.h
        lib/LPC/LPC_huber_stat.cpp
        lib/Math/Bairstow.cpp
        lib/Math/Bairstow.h
        lib/Math/Polynomial.cpp
        lib/Math/Polynomial.h
        lib/Math/Viterbi.cpp
        lib/Math/Viterbi.h
        lib/Pitch/McLeod/autocorrelation.cpp
        lib/Pitch/McLeod/parabolic_interpolation.cpp
        lib/Pitch/McLeod/peak_picking.cpp
        lib/Pitch/McLeod/MPM.h
        lib/Pitch/Pitch_AMDF.cpp
        lib/Pitch/Pitch_MPM.cpp
        lib/Pitch/Pitch.h
        lib/Signal/Filter.cpp
        lib/Signal/Filter.h
        lib/Signal/Resample.cpp
        lib/Signal/Resample.h
        lib/Signal/Window.cpp
        lib/Signal/Window.h
        lib/FFT/FFT.cpp
        lib/FFT/FFT.h
        lib/MFCC/MFCC.cpp
        lib/MFCC/MFCC.h)

set(QT_SOURCES
        ../resources.qrc)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(PortAudio REQUIRED)
find_package(FFTW REQUIRED DOUBLE_LIB)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

set(CMAKE_FIND_ROOT_PATH "${TORCH_INSTALL_DIR}")
find_package(Torch REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/libs
        ${EIGEN_INCLUDE_DIR}
        ${PORTAUDIO_INCLUDE_DIRS}
        ${FFTW_INCLUDE_DIRS}
        ${TORCH_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
)

add_executable(speech_analysis WIN32 ${SOURCES} ${QT_SOURCES})

target_link_libraries(speech_analysis
        Eigen3::Eigen
        ${PORTAUDIO_LIBRARIES}
        ${FFTW_LIBRARIES}
        ${TORCH_LIBRARY}
        Qt5::Widgets
        OpenGL::GL)

if (WIN32)
add_custom_command (TARGET speech_analysis POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:speech_analysis>)
endif()
