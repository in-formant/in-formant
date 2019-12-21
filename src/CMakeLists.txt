cmake_minimum_required(VERSION 3.0)
project(main)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-fopenmp -fext-numeric-literals")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
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
        analysis/parts/median.cpp
        analysis/parts/pitch.cpp
        analysis/parts/preprocess.cpp
        analysis/parts/resample.cpp
        analysis/parts/spectrum.cpp
        audio/AudioCapture.cpp
        audio/AudioCapture_callbacks.cpp
        audio/AudioDevices.cpp
        audio/AudioDevices.h
        audio/RingBuffer.cpp
        audio/RingBuffer.h
        gui/canvas/AnalyserCanvas.cpp
        gui/canvas/AnalyserCanvas.h
        gui/qt/MainWindow.cpp
        gui/qt/MainWindow.h)

set(QT_SOURCES
       ../resources.qrc)

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(PortAudio REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

include_directories(
        ${EIGEN_INCLUDE_DIR}
        ${PORTAUDIO_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${FFTW3_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIRS}
        ${libspeech_INCLUDE_DIR}
)

add_executable(speech_analysis WIN32 ${SOURCES} ${QT_SOURCES})

target_link_libraries(speech_analysis
        ${libspeech_LIBRARY}
        Eigen3::Eigen
        ${PORTAUDIO_LIBRARIES}
        ${FFTW3_LIBRARY}
        Qt5::Widgets
        OpenGL::GL)

if (WIN32)
add_custom_command (TARGET speech_analysis POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:speech_analysis>)
endif()
