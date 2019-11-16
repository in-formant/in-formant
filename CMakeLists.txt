cmake_minimum_required(VERSION 3.15)
project(speech_analysis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${speech_analysis_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS_RELEASE "-Os -ftree-vectorize -s")

set(SOURCES
        src/main.cpp
        src/Exceptions.cpp
        src/Exceptions.h
        src/analysis/Analyser_mainLoop.cpp
        src/analysis/Analyser.cpp
        src/analysis/Analyser.h
        src/audio/AudioCapture.cpp
        src/audio/AudioCapture_callbacks.cpp
        src/audio/AudioCapture.h
        src/audio/RingBuffer.cpp
        src/audio/RingBuffer.h
        src/gui/AnalyserWindow.cpp
        src/gui/AnalyserWindow.h
        src/gui/SDLUtils.cpp
        src/gui/SDLUtils.h
        src/gui/Spectrogram.cpp
        src/gui/Spectrogram.h
        src/lib/FFT/FFT_core.h
        src/lib/FFT/FFT_d.cpp
        src/lib/FFT/FFT.h
        src/lib/Formant/Formant.cpp
        src/lib/Formant/Formant.h
        src/lib/Formant/Formant_track.cpp
        src/lib/LPC/Frame/LPC_Frame.cpp
        src/lib/LPC/Frame/LPC_Frame.h
        src/lib/LPC/Frame/LPC_Frame_auto.cpp
        src/lib/LPC/Frame/LPC_Frame_burg.cpp
        src/lib/LPC/Frame/LPC_Frame_covar.cpp
        src/lib/LPC/Frame/LPC_Frame_huber.cpp
        src/lib/LPC/LPC.cpp
        src/lib/LPC/LPC.h
        src/lib/LPC/LPC_filter.cpp
        src/lib/LPC/LPC_huber.cpp
        src/lib/LPC/LPC_huber.h
        src/lib/LPC/LPC_huber_stat.cpp
        src/lib/Math/Bairstow.cpp
        src/lib/Math/Bairstow.h
        src/lib/Math/Polynomial.cpp
        src/lib/Math/Polynomial.h
        src/lib/Math/Viterbi.cpp
        src/lib/Math/Viterbi.h
        src/lib/Pitch/YAAPT/util/fir1.cpp
        src/lib/Pitch/YAAPT/util/fir2.cpp
        src/lib/Pitch/YAAPT/util/interp1.cpp
        src/lib/Pitch/YAAPT/util/medfilt1.cpp
        src/lib/Pitch/YAAPT/util/specgram.cpp
        src/lib/Pitch/YAAPT/cmp_rate.cpp
        src/lib/Pitch/YAAPT/crs_corr.cpp
        src/lib/Pitch/YAAPT/dynamic5.cpp
        src/lib/Pitch/YAAPT/dynamic.cpp
        src/lib/Pitch/YAAPT/nlfer.cpp
        src/lib/Pitch/YAAPT/nonlinear.cpp
        src/lib/Pitch/YAAPT/path1.cpp
        src/lib/Pitch/YAAPT/peaks.cpp
        src/lib/Pitch/YAAPT/refine.cpp
        src/lib/Pitch/YAAPT/tm_trk.cpp
        src/lib/Pitch/YAAPT/spec_trk.cpp
        src/lib/Pitch/YAAPT/YAAPT.cpp
        src/lib/Pitch/YAAPT/YAAPT.h
        src/lib/Pitch/Pitch_AMDF.cpp
        src/lib/Pitch/Pitch.h
        src/lib/Signal/Filter.cpp
        src/lib/Signal/Filter.h
        src/lib/Signal/Resample.cpp
        src/lib/Signal/Resample.h
        src/lib/Signal/Window.cpp
        src/lib/Signal/Window.h src/analysis/parts/pitch.cpp src/analysis/parts/resample.cpp src/analysis/parts/lpc.cpp src/analysis/parts/formants.cpp)

add_executable(speech_analysis ${SOURCES})

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)
find_package(PortAudio REQUIRED)
find_package(FFTW REQUIRED DOUBLE_LIB)

include_directories(
        ${SDL2_INCLUDE_DIR}
        ${SDL2_TTF_INCLUDE_DIR}
        ${SDL2_GFX_INCLUDE_DIRS}
        ${EIGEN_INCLUDE_DIR}
        ${PORTAUDIO_INCLUDE_DIRS}
        ${FFTW_INCLUDE_DIRS})

target_link_libraries(speech_analysis
        ${SDL2_LIBRARY}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_GFX_LIBRARIES}
        ${SDL2_LIBRARY}
        Eigen3::Eigen
        ${PORTAUDIO_LIBRARIES}
        ${FFTW_LIBRARIES})
