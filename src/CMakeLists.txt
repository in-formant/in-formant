cmake_minimum_required(VERSION 3.0)
project(main)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-fext-numeric-literals -ftree-vectorize")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g3")

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

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(portaudio REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(FFTW3 REQUIRED)

include_directories(
	${libspeech_INCLUDE_DIR}
	${INCLUDE_DIRS}
)

add_executable(speech_analysis WIN32 ${SOURCES} ${QT_SOURCES})

target_link_libraries(speech_analysis
    ${libspeech_LIBRARY}
    Eigen3::Eigen
	portaudio_static
	FFTW3::fftw3
	Qt5::Widgets
)

if (WIN32)
add_custom_command (TARGET speech_analysis POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:speech_analysis>)
endif()
