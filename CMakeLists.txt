cmake_minimum_required(VERSION 3.0)
project(speech_analysis)

include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

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

add_dependencies(main libspeech)
add_dependencies(speech_analysis main)
