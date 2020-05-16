cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(speech_analysis VERSION 1.5.7)

include(ExternalProject)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if(ANDROID)
    set(libspeech_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(libspeech_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(sa_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    set(libspeech_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(libspeech_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(sa_SUFFIX ${CMAKE_EXECUTABLE_SUFFIX})
endif()

if (ANDROID)
    set(VERSION_ABI_SEP "-")
    if (ANDROID_ABI MATCHES "^armeabi-v7a$")
        set(VERSION_ABI 1)
    elseif (ANDROID_ABI MATCHES "^arm64-v8a$")
        set(VERSION_ABI 2)
    elseif (ANDROID_ABI MATCHES "^x86$")
        set(VERSION_ABI 3)
    elseif (ANDROID_ABI MATCHES "^x86_64$")
        set(VERSION_ABI 4)
    endif()
else()
    set(VERSION_ABI_SEP)
    set(VERSION_ABI)
    set(ANDROID_ABI)
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

leftpadzeroes(VERSION_MAJOR_PAD ${VERSION_MAJOR})
leftpadzeroes(VERSION_MINOR_PAD ${VERSION_MINOR})
leftpadzeroes(VERSION_PATCH_PAD ${VERSION_PATCH})

set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_ABI_SEP}${ANDROID_ABI})
set(VERSION_CODE ${VERSION_ABI}${VERSION_MAJOR_PAD}${VERSION_MINOR_PAD}${VERSION_PATCH_PAD})

set(libspeech_LIBRARY ${CMAKE_BINARY_DIR}/libspeech/src/libspeech-build/${libspeech_PREFIX}speech${libspeech_SUFFIX})
set(sa_EXECUTABLE ${CMAKE_BINARY_DIR}/speech_analysis/src/speech_analysis-build/speech_analysis${sa_SUFFIX})

set(CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -Dlibspeech_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/lib
    -Dlibspeech_LIBRARY=${libspeech_LIBRARY}
   
    -DVERSION_STRING=${VERSION_STRING}
    -DVERSION_CODE=${VERSION_CODE}

    -DANDROID_ABI=${ANDROID_ABI}
    -DANDROID_PLATFORM=${ANDROID_PLATFORM}
    -DEigen3_DIR=${Eigen3_DIR}
    -DFFTW_ROOT=${FFTW_ROOT}
    -DTorch_DIR=${Torch_DIR}
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

ExternalProject_Add(speech_analysis
    PREFIX ${CMAKE_BINARY_DIR}/speech_analysis
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
    BUILD_ALWAYS 1
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_COMMAND $ENV{CROSS}cmake
	CMAKE_ARGS ${CMAKE_ARGS}
)

add_dependencies(speech_analysis libspeech)

if (UNIX AND NOT ANDROID AND NOT APPLE)
    install(FILES ${libspeech_LIBRARY} DESTINATION lib)
    install(PROGRAMS ${sa_EXECUTABLE} DESTINATION bin)
    install(CODE "
        file(GET_RUNTIME_DEPENDENCIES
            RESOLVED_DEPENDENCIES_VAR LIB_FILES
            EXECUTABLES ${sa_EXECUTABLE}
            LIBRARIES ${libspeech_LIBRARY}
        )
        file(INSTALL \${LIB_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib FOLLOW_SYMLINK_CHAIN)
    ")
endif()

if (WIN32)
    install(FILES ${libspeech_LIBRARY} DESTINATION .)
    install(PROGRAMS ${sa_EXECUTABLE} DESTINATION .)
    install(CODE "
        execute_process(COMMAND
            /usr/mxe/tools/copydlldeps.sh
            --infile ${sa_EXECUTABLE}
            --infile ${libspeech_LIBRARY}
            --destdir \$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}
            --recursivesrcdir /usr/mxe/usr/$ENV{CROSS::-1}
            --copy --objdump /usr/mxe/usr/bin/$ENV{CROSS}objdump
        )
    ")
endif()
