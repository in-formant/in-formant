cmake_minimum_required(VERSION 3.0)
project(speech_analysis)

include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(APPLE)
    set(TORCH_URL https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.3.1.zip)
elseif(UNIX)
    set(TORCH_URL https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.3.1%2Bcpu.zip)
elseif(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(TORCH_URL https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-debug-1.3.1.zip)
    else()
        set(TORCH_URL https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.3.1.zip)
    endif()
endif()

set(TORCH_INSTALL_DIR ${CMAKE_BINARY_DIR}/libtorch)

ExternalProject_Add(torch
        PREFIX ${CMAKE_BINARY_DIR}/libtorch-log
        SOURCE_DIR ${TORCH_INSTALL_DIR}
        INSTALL_DIR ${TORCH_INSTALL_DIR}
        URL ${TORCH_URL}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        PATCH_COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/patch/TorchConfig.cmake ${TORCH_INSTALL_DIR}/share/cmake/Torch/TorchConfig.cmake
)

ExternalProject_Add(main
        PREFIX ${CMAKE_BINARY_DIR}/speech_analysis
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
        BUILD_ALWAYS 1
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        CMAKE_COMMAND $ENV{CROSS}cmake
        CMAKE_ARGS -DTORCH_INSTALL_DIR=${TORCH_INSTALL_DIR} -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/cmake
)

add_custom_target(speech_analysis)

add_dependencies(main torch)
add_dependencies(speech_analysis main)
