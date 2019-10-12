cmake_minimum_required(VERSION 3.15)
project(speech_analysis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${speech_analysis_SOURCE_DIR}/cmake")

set(SOURCES
        src/main.cpp
        src/gui/AnalyserWindow.cpp
        src/gui/AnalyserWindow.h
        src/Exceptions.cpp
        src/Exceptions.h
        src/gui/SDLUtils.cpp
        src/gui/SDLUtils.h
        src/audio/AudioCapture.cpp
        src/audio/AudioCapture_callbacks.cpp
        src/audio/AudioCapture.h
        src/audio/RingBuffer.cpp
        src/audio/RingBuffer.h
        src/gui/Spectrogram.cpp
        src/gui/Spectrogram.h
        src/signal/Pitch.h
        src/signal/Pitch_AMDF.cpp
        src/signal/Filter.cpp
        src/signal/Filter.h
        src/signal/Window.cpp
        src/signal/Window.h
        src/signal/Polynomial_impl.h
        src/signal/Polynomial.h
        src/signal/Resample.cpp
        src/signal/Resample.h
        src/signal/FFT.cpp
        src/signal/FFT.h
        src/signal/FFT_core.h
        src/signal/LPC_filter.cpp
        src/signal/LPC_Frame_auto.cpp
        src/signal/LPC_Frame_covar.cpp
        src/signal/LPC_Frame_huber.cpp
        src/signal/LPC_Frame.h
        src/signal/LPC_huber_stat.cpp
        src/signal/LPC_huber.h
        src/signal/LPC.cpp
        src/signal/LPC_huber.cpp
        src/signal/LPC.h)

add_executable(speech_analysis ${SOURCES})

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SoundIo REQUIRED)

include_directories(
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${SDL2_TTF_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${SOUNDIO_INCLUDE_DIR})

target_link_libraries(speech_analysis
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        Eigen3::Eigen
        ${SOUNDIO_LIBRARY})
