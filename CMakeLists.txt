cmake_minimum_required(VERSION 3.0)
project(speech_analysis)

include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/deps")

set(DEPS_PREFIX ${CMAKE_BINARY_DIR}/deps)
set(Eigen3_DIR ${DEPS_PREFIX}/share/eigen3/cmake)

set(FFTW3_INCLUDE_DIR ${DEPS_PREFIX}/include)
set(FFTW3_LIBRARY ${DEPS_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fftw3${CMAKE_STATIC_LIBRARY_SUFFIX})

set(libspeech_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)
set(libspeech_LIBRARY ${CMAKE_BINARY_DIR}/libspeech/src/libspeech-build/${CMAKE_STATIC_LIBRARY_PREFIX}speech${CMAKE_STATIC_LIBRARY_SUFFIX})

set(CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DEigen3_DIR=${Eigen3_DIR}
    -DFFTW3_INCLUDE_DIR=${FFTW3_INCLUDE_DIR}
    -DFFTW3_LIBRARY=${FFTW3_LIBRARY}
    -Dlibspeech_INCLUDE_DIR=${libspeech_INCLUDE_DIR}
    -Dlibspeech_LIBRARY=${libspeech_LIBRARY}
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
        CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/cmake
)

set(libspeech_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)
set(libspeech_LIBRARY ${CMAKE_BINARY_DIR}/libspeech/src/libspeech-build/${CMAKE_STATIC_LIBRARY_PREFIX}speech${CMAKE_STATIC_LIBRARY_SUFFIX})

ExternalProject_Add(main
        PREFIX ${CMAKE_BINARY_DIR}/speech_analysis
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
        BUILD_ALWAYS 1
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        CMAKE_COMMAND $ENV{CROSS}cmake
        CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/cmake -Dlibspeech_INCLUDE_DIR=${libspeech_INCLUDE_DIR} -Dlibspeech_LIBRARY=${libspeech_LIBRARY}
)

add_custom_target(speech_analysis)

add_dependencies(libspeech eigen3 fftw3)
add_dependencies(main libspeech eigen3 fftw3)
add_dependencies(speech_analysis main)
