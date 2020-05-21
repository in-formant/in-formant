cmake_minimum_required(VERSION 3.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
project(speech_analysis)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "-fPIC -mtune=generic -Wall -pedantic -pthread")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g3")

set(CMAKE_CXX_FLAGS "-fPIC -mtune=generic -Wall -pedantic -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3")

if(EMSCRIPTEN)
    set(EMSCRIPTEN_ARGS
        "--bind"
        "-s WASM=1"
        "-s ASYNCIFY=1"
        "-s TOTAL_MEMORY=512MB"
        "-s FULL_ES2=1"
        "-s USE_WEBGL2=1"
        "-s \"EXTRA_EXPORTED_RUNTIME_METHODS=['UTF16ToString', 'stringToUTF16']\""
        "-s USE_PTHREADS=1"
        "-s FORCE_FILESYSTEM=1"
        "-s FILESYSTEM=1"
        "-s NO_EXIT_RUNTIME=0"
        "-Wcast-align"
        "-Wover-aligned"
        "-s WARN_UNALIGNED=1"
        "-mnontrapping-fptoint"
        "-flto"
        "-s DISABLE_EXCEPTION_CATCHING=1"
    )
    add_definitions(${EMSCRIPTEN_ARGS})
    link_libraries(${EMSCRIPTEN_ARGS})
endif()

add_definitions(-DVERSION_STRING=\"${VERSION_STRING}\" -DVERSION_CODE=${VERSION_CODE})

set(SOURCES
    main.cpp
    Exceptions.cpp
    Exceptions.h
    log/simpleQtLogger.cpp
    log/simpleQtLogger.h
    time/time_util.cc
    time/time_util.h
    analysis/Analyser_mainLoop.cpp
    analysis/Analyser.cpp
    analysis/Analyser.h
    analysis/parts/formants.cpp
    analysis/parts/lpc.cpp
    analysis/parts/smooth.cpp
    analysis/parts/pitch.cpp
    analysis/parts/deepformants.cpp
    analysis/parts/openquotient.cpp
    analysis/parts/inversefilter.cpp
    analysis/parts/preprocess.cpp
    analysis/parts/resample.cpp
    analysis/parts/spectrum.cpp
    analysis/parts/track.cpp
    audio/AudioInterface.h
    audio/AudioInterface.cpp
    audio/AudioInterface_callbacks.cpp
    audio/AudioDevices.cpp
    audio/AudioDevices.h
    audio/RingBuffer.cpp
    audio/RingBuffer.h
    audio/SineWave.cpp
    audio/SineWave.h
    audio/NoiseFilter.cpp
    audio/NoiseFilter.h
    audio/miniaudio.h
    audio/implementation.cpp
    gui/ColorMaps.cpp
    gui/ColorMaps.h
    gui/Keybinds.cpp
    gui/Keybinds.h
    gui/AnalyserCanvas.cpp
    gui/AnalyserCanvas.h
    gui/PowerSpectrum.cpp
    gui/PowerSpectrum.h
    gui/Oscilloscope.cpp
    gui/Oscilloscope.h
    gui/ui_MainWindow.cpp
    gui/MainWindow.cpp
    gui/MainWindow.h
    ../resources.qrc)

set(ANDROID_JNI_SOURCES
    jni/JniInstance.h
    jni/JniInstance.cpp
    jni/bindings.cpp)

set(EM_SOURCES qwasmsettings.h)

if (APPLE)
    set(QT_PREFIX "$ENV{OSXCROSS}/macports/pkgs/opt/local/libexec/qt5")
    set(PKG_CONFIG_EXECUTABLE "$ENV{OSXCROSS}/bin/$ENV{CROSS}pkg-config")
    unset(ENV{PKG_CONFIG_LIBDIR})
    unset(ENV{PKG_CONFIG_SYSROOT_DIR})
    set(ENV{OSXCROSS_PKG_CONFIG_LIBDIR} "${QT_PREFIX}/lib/pkgconfig")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Qt5Widgets REQUIRED Qt5Widgets)
elseif (NOT ANDROID AND NOT EMSCRIPTEN)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

find_package(Eigen3 REQUIRED NO_MODULE)

if (FFTW_ROOT)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
endif()

find_package(FFTW REQUIRED COMPONENTS DOUBLE_LIB)

include_directories(
    ${libspeech_INCLUDE_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${EIGEN_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIRS}
)

if (APPLE OR ANDROID OR EMSCRIPTEN)
    if (APPLE)
        set(MOC_EXE moc)
        set(RCC_EXE rcc)
    else()
        if (ANDROID)
            set(QT_PREFIX /usr/android)
        elseif (EMSCRIPTEN)
            set(QT_PREFIX /em-target)
        endif()
        
        set(MOC_EXE ${QT_PREFIX}/bin/moc)
        set(RCC_EXE ${QT_PREFIX}/bin/rcc)
    endif()

    foreach (src ${SOURCES})

        get_filename_component(dir "${src}" DIRECTORY)
        get_filename_component(name "${src}" NAME_WE)
        get_filename_component(ext "${src}" EXT)

        if (${ext} MATCHES ".qrc")
            set(outfile "${CMAKE_BINARY_DIR}/qrc_${name}.cpp")
            execute_process(COMMAND ${RCC_EXE} ${src} -o ${outfile} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
            list(APPEND SOURCES ${outfile})
        elseif (${ext} MATCHES ".cpp" OR ${ext} MATCHES ".h")
            file(READ "${src}" content)
            foreach (macro_name ${CMAKE_AUTOMOC_MACRO_NAMES})
                string(FIND "${content}" "${macro_name}" match_res)
                if (NOT ${match_res} EQUAL -1)
                    set(outfile "${CMAKE_BINARY_DIR}/moc_${name}.cpp")
                    execute_process(COMMAND ${MOC_EXE} ${COMPILE_DEFINITIONS} ${src} -o ${outfile} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
                    list(APPEND SOURCES ${outfile})
                endif()
            endforeach()
        endif()

    endforeach()
endif()

if (ANDROID)
    add_library(speech_analysis SHARED ${SOURCES} ${ANDROID_JNI_SOURCES})
    set_property(TARGET speech_analysis PROPERTY OUTPUT_NAME speech_analysis_${ANDROID_ABI})

    set(QT_ANDROID_SDK_ROOT /opt/android-sdk)
    set(QT_ANDROID_NDK_ROOT /opt/android-ndk/android-ndk-r21)
    set(Qt5Core_DIR ${QT_PREFIX}/lib/cmake/Qt5Core)
    set(ANDROID_STL c++_shared)
    include(/usr/qt-android-cmake/AddQtAndroidApk.cmake)

    file(READ ${CMAKE_SOURCE_DIR}/../android/keystore.password KEYSTORE_PASSWORD)
    add_qt_android_apk(apk speech_analysis
        NAME "Speech analysis"
        VERSION_CODE ${VERSION_CODE}
        PACKAGE_NAME "fr.cloyunhee.speechanalysis"
        PACKAGE_SOURCES ${CMAKE_SOURCE_DIR}/../android
        KEYSTORE ${CMAKE_SOURCE_DIR}/../android/keystore.jks speechanalysis-key-alias
        KEYSTORE_PASSWORD ${KEYSTORE_PASSWORD}
        DEPENDS ${libspeech_LIBRARY} ${FFTW_LIBRARIES}
    )
elseif (EMSCRIPTEN)
    add_executable(speech_analysis ${SOURCES} ${EM_SOURCES})
else()
    add_executable(speech_analysis WIN32 ${SOURCES})
endif()

target_link_libraries(speech_analysis
    ${libspeech_LIBRARY}
    Eigen3::Eigen
    ${FFTW_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Widgets_PLUGINS}
)

if (APPLE)
    target_link_libraries(speech_analysis
        -F${QT_PREFIX}/lib
        "-framework QtWidgets"
        "-framework QtGui"
        "-framework QtCore"
        "-rpath @executable_path/."
        ${QT_PREFIX}/plugins/platforms/libqcocoa.dylib
    )
elseif (UNIX)
    target_link_libraries(speech_analysis dl m)
endif()

if(EMSCRIPTEN)
    include_directories(
        ${QT_PREFIX}/include
        ${QT_PREFIX}/include/QtCore
        ${QT_PREFIX}/include/QtGui
        ${QT_PREFIX}/include/QtWidgets
    )
    target_link_libraries(speech_analysis
        -L${QT_PREFIX}/lib
        -L${QT_PREFIX}/plugins
        -L${QT_PREFIX}/plugins/platforms
        -L${QT_PREFIX}/plugins/imageformats
        qwasm
        Qt5EventDispatcherSupport
        Qt5FontDatabaseSupport
        qtfreetype
        Qt5EglSupport
        libqgif
        libqico
        libqjpeg
        Qt5Widgets
        Qt5Gui
        qtlibpng
        qtharfbuzz
        Qt5Core
        qtpcre2
    )
elseif(ANDROID)
    set(libsuff "_${ANDROID_ABI}")
    set(imforpre "plugins_imageformats_")
    set(platfpre "plugins_platforms_")
    include_directories(
        ${QT_PREFIX}/include
        ${QT_PREFIX}/include/QtCore
        ${QT_PREFIX}/include/QtGui
        ${QT_PREFIX}/include/QtWidgets
        ${QT_PREFIX}/include/QtAndroidExtras
    )
    target_link_libraries(speech_analysis
        -L${SYSROOT}/usr/lib
        -L${QT_PREFIX}/${ANDROID_ABI}/lib
        -L${QT_PREFIX}/lib
        -L${QT_PREFIX}/plugins
        -L${QT_PREFIX}/plugins/platforms
        -L${QT_PREFIX}/plugins/imageformats
        ${platfpre}qtforandroid${libsuff}
        Qt5EventDispatcherSupport${libsuff}
        Qt5EglSupport${libsuff}
        Qt5VulkanSupport${libsuff}
        Qt5AccessibilitySupport${libsuff}
        Qt5ServiceSupport${libsuff}
        Qt5ThemeSupport${libsuff}
        Qt5FontDatabaseSupport${libsuff}
        qtfreetype${libsuff}
        Qt5EdidSupport${libsuff}
        Qt5AndroidExtras${libsuff}
        ${imforpre}qgif${libsuff}
        ${imforpre}qico${libsuff}
        ${imforpre}qjpeg${libsuff}
        Qt5Widgets${libsuff}
        Qt5Gui${libsuff}
        qtlibpng${libsuff}
        #qtharfbuzz${libsuff}
        Qt5Core${libsuff}
        #qtpcre2${libsuff}
        z
        log
        EGL
        GLESv2
        OpenSLES
        android
        jnigraphics
        dl
    )
else()
    message("Target system not supported")
endif()
