cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(speech_analysis VERSION 1.5.2)

include(ExternalProject)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if(ANDROID)
    set(libspeech_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    set(libspeech_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_PATCH ${PROJECT_VERSION_PATCH})

function(leftpadzeroes out_var value)
    string(LENGTH "${value}" value_length)
    if (value_length EQUAL 1)
        set(${out_var} "0${value}" PARENT_SCOPE)
    else()
        set(${out_var} "${value}" PARENT_SCOPE)
    endif()
endfunction()

leftpadzeroes(VERSION_MINOR_PAD ${VERSION_MINOR})
leftpadzeroes(VERSION_PATCH_PAD ${VERSION_PATCH})

set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(VERSION_CODE ${VERSION_MAJOR}${VERSION_MINOR_PAD}${VERSION_PATCH_PAD})

set(CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -Dlibspeech_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/lib
    -Dlibspeech_LIBRARY=${CMAKE_BINARY_DIR}/libspeech/src/libspeech-build/${CMAKE_STATIC_LIBRARY_PREFIX}speech${libspeech_SUFFIX}
   
    -DVERSION_STRING=${VERSION_STRING}
    -DVERSION_CODE=${VERSION_CODE}

    -DANDROID_ABI=${ANDROID_ABI}
    -DANDROID_PLATFORM=${ANDROID_PLATFORM}
    -DEigen3_DIR=${Eigen3_DIR}
    -DFFTW_ROOT=${FFTW_ROOT}
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

add_dependencies(main libspeech)
add_dependencies(speech_analysis main)
