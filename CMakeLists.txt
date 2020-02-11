cmake_minimum_required(VERSION 3.0)
project(speech_analysis)

include(ExternalProject)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/deps")

set(DEPS_PREFIX ${CMAKE_BINARY_DIR}/deps)

set(CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DEigen3_DIR=${DEPS_PREFIX}/share/eigen3/cmake
    -DINCLUDE_DIRS=${DEPS_PREFIX}/include;${DEPS_PREFIX}/include/eigen3
    -Dlibspeech_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/lib
    -Dlibspeech_LIBRARY=${CMAKE_BINARY_DIR}/libspeech/src/libspeech-build/${CMAKE_STATIC_LIBRARY_PREFIX}speech${CMAKE_STATIC_LIBRARY_SUFFIX}
)

ExternalProject_Add(eigen3
    PREFIX ${CMAKE_BINARY_DIR}/eigen3
    URL https://gitlab.com/libeigen/eigen/-/archive/7252163335f56f23fcc7381c1efdea47161005fa/eigen-7252163335f56f23fcc7381c1efdea47161005fa.tar.bz2
    URL_HASH SHA256=bbb89ecda2d27f854cea0bc6da7a472ccee12424ad14da91cd367e6f46e8070c
	CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${DEPS_PREFIX}
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

add_dependencies(libspeech eigen3)
add_dependencies(main eigen3 libspeech)
add_dependencies(speech_analysis main)
