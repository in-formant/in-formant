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
        src/lib/Pitch/Pitch.h
        src/lib/Pitch/Pitch_AMDF.cpp
        src/lib/Signal/Filter.cpp
        src/lib/Signal/Filter.h
        src/lib/Signal/Window.cpp
        src/lib/Signal/Window.h
        src/lib/Math/Polynomial.h
        src/lib/Signal/Resample.cpp
        src/lib/Signal/Resample.h
        src/lib/FFT/FFT.cpp
        src/lib/FFT/FFT.h
        src/lib/FFT/FFT_core.h
        src/lib/LPC/LPC_filter.cpp
        src/lib/LPC/Frame/LPC_Frame_auto.cpp
        src/lib/LPC/Frame/LPC_Frame_covar.cpp
        src/lib/LPC/Frame/LPC_Frame_burg.cpp
        src/lib/LPC/Frame/LPC_Frame_huber.cpp
        src/lib/LPC/Frame/LPC_Frame.h
        src/lib/LPC/LPC_huber_stat.cpp
        src/lib/LPC/LPC_huber.h
        src/lib/LPC/LPC.cpp
        src/lib/LPC/LPC_huber.cpp
        src/lib/LPC/LPC.h src/lib/LPC/Frame/LPC_Frame.cpp src/lib/Formant/Formant.cpp src/lib/Formant/Formant.h src/lib/Math/Polynomial.cpp)

add_executable(speech_analysis ${SOURCES})

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(PortAudio REQUIRED)

include_directories(
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${SDL2_TTF_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${PORTAUDIO_INCLUDE_DIRS})

target_link_libraries(speech_analysis
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        Eigen3::Eigen
        ${PORTAUDIO_LIBRARIES})
