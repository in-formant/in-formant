cmake_minimum_required(VERSION 3.0)
project(speech_analysis)

include(ExternalProject)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/deps")

set(DEPS_PREFIX ${CMAKE_BINARY_DIR}/deps)

set(CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -Dportaudio_DIR=${DEPS_PREFIX}/lib/pkgconfig
    -DQt5_DIR=${DEPS_PREFIX}/lib/cmake/Qt5
    -DEigen3_DIR=${DEPS_PREFIX}/share/eigen3/cmake
    -DFFTW3_DIR=${DEPS_PREFIX}/lib/cmake/fftw3
    -DINCLUDE_DIRS=${DEPS_PREFIX}/include;${DEPS_PREFIX}/include/eigen3
    -Dlibspeech_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/lib
    -Dlibspeech_LIBRARY=${CMAKE_BINARY_DIR}/libspeech/src/libspeech-build/${CMAKE_STATIC_LIBRARY_PREFIX}speech${CMAKE_STATIC_LIBRARY_SUFFIX}
)

ExternalProject_Add(qt5
	PREFIX ${CMAKE_BINARY_DIR}/qt5
	GIT_REPOSITORY https://code.qt.io/qt/qt5.git
	GIT_TAG v5.14.0
	GIT_SUBMODULES qtbase
	GIT_PROGRESS TRUE
	BUILD_IN_SOURCE 1
	UPDATE_COMMAND ""
	CONFIGURE_COMMAND ./configure -opensource -confirm-license -static -qt-zlib -qt-libjpeg -qt-libpng -qt-xcb -qt-freetype -qt-pcre -qt-harfbuzz -no-opengl -prefix ${DEPS_PREFIX}
	BUILD_COMMAND make -j 4
)

ExternalProject_Add(portaudio
	PREFIX ${CMAKE_BINARY_DIR}/portaudio
	URL http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
	URL_HASH SHA256=f5a21d7dcd6ee84397446fa1fa1a0675bb2e8a4a6dceb4305a8404698d8d1513 
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/patch/portaudio ${CMAKE_BINARY_DIR}/portaudio/src/portaudio
    CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${DEPS_PREFIX} -DPA_BUILD_STATIC=ON -DPA_BUILD_SHARED=OFF -DPA_USE_ASIO=ON -DPA_USE_DS=ON -DPA_USE_WMME=ON -DPA_USE_WASAPI=ON -DPA_USE_WDMKS=ON -DPA_USE_WDMKS_DEVICE_INFO=ON -DPA_USE_COREAUDIO=ON -DPA_USE_JACK=ON -DPA_USE_ALSA=ON -DPA_UNICODE_BUILD=ON
)

ExternalProject_Add(eigen3
        PREFIX ${CMAKE_BINARY_DIR}/eigen3
        URL https://gitlab.com/libeigen/eigen/-/archive/7252163335f56f23fcc7381c1efdea47161005fa/eigen-7252163335f56f23fcc7381c1efdea47161005fa.tar.bz2
        URL_HASH SHA256=bbb89ecda2d27f854cea0bc6da7a472ccee12424ad14da91cd367e6f46e8070c
	CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${DEPS_PREFIX}
)

ExternalProject_Add(fftw3
        PREFIX ${CMAKE_BINARY_DIR}/fftw3
        URL http://www.fftw.org/fftw-3.3.8.tar.gz
        URL_HASH SHA256=6113262f6e92c5bd474f2875fa1b01054c4ad5040f6b0da7c03c98821d9ae303
	CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${DEPS_PREFIX} -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=OFF
)

ExternalProject_Add(libspeech
        PREFIX ${CMAKE_BINARY_DIR}/libspeech
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib
        BUILD_ALWAYS 1
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        CMAKE_COMMAND $ENV{CROSS}cmake
	CMAKE_ARGS ${CMAKE_ARGS}
)

ExternalProject_Add(main
        PREFIX ${CMAKE_BINARY_DIR}/speech_analysis
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
        BUILD_ALWAYS 1
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        CMAKE_COMMAND $ENV{CROSS}cmake
	CMAKE_ARGS ${CMAKE_ARGS}
)

add_custom_target(speech_analysis)

add_dependencies(libspeech eigen3 fftw3)
add_dependencies(main qt5 portaudio eigen3 fftw3 libspeech)
add_dependencies(speech_analysis main)
