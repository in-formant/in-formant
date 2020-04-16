cmake_minimum_required(VERSION 3.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
project(main)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-fPIC -mtune=generic -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g0")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    qt_plugin.cpp
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
    analysis/parts/openquotient.cpp
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
    gui/AnalyserCanvas.cpp
    gui/AnalyserCanvas.h
    gui/PowerSpectrum.cpp
    gui/PowerSpectrum.h
    gui/MainWindow.cpp
    gui/MainWindow.h
    ../resources.qrc)

set(ANDROID_JNI_SOURCES
    jni/JniInstance.h
    jni/JniInstance.cpp
    jni/bindings.cpp)

if (NOT APPLE AND NOT ANDROID)
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

if (APPLE OR ANDROID)
    if (APPLE)
        set(QT_PREFIX $ENV{OSXCROSS}/macports/pkgs/opt/local)
    elseif (ANDROID)
        set(QT_PREFIX /usr/android)
    endif()
    
    set(MOC_EXE ${QT_PREFIX}/bin/moc)
    set(RCC_EXE ${QT_PREFIX}/bin/rcc)

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
                    execute_process(COMMAND ${MOC_EXE} ${src} -o ${outfile} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
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

    message(STATUS "Version string: ${VERSION_STRING}")
    message(STATUS "Version code: ${VERSION_CODE}")

    add_definitions(-DVERSION_STRING=\"${VERSION_STRING}\" -DVERSION_CODE=${VERSION_CODE})

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
else()
    add_executable(speech_analysis WIN32 ${SOURCES})
endif()

target_link_libraries(speech_analysis
    ${libspeech_LIBRARY}
    Eigen3::Eigen
    ${FFTW_LIBRARIES}
)

if(APPLE)
    set(SDK_PREFIX $ENV{OSXCROSS}/SDK/MacOSX10.13.sdk)
    include_directories(
        ${SDK_PREFIX}/usr/include/c++/v1
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
        -F${SDK_PREFIX}/System/Library/Frameworks
        "-framework AppKit"
        "-framework Carbon"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreText"
        "-framework CoreServices"
        "-framework CoreVideo"
        "-framework Foundation"
        "-framework ImageIO"
        "-framework IOKit"
        "-framework IOSurface"
        "-framework Metal"
        "-framework OpenGL"
        "-framework QuartzCore"
        "-framework Security"
        "-framework System"
        qminimal
        Qt5AccessibilitySupport
        Qt5ClipboardSupport
        Qt5ThemeSupport
        Qt5FontDatabaseSupport
        Qt5GraphicsSupport
        Qt5PrintSupport
        cups
        qcocoa
        qtfreetype
        Qt5DBus
        qgif
        qico
        qjpeg
        Qt5Widgets
        Qt5Gui
        qtlibpng
        qtharfbuzz
        Qt5Core
        m
        qtpcre2
        dl
        pthread
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
elseif(UNIX)
    target_link_libraries(speech_analysis
        /usr/plugins/platforms/libqminimal.a
        Qt5EventDispatcherSupport
        /usr/plugins/platforms/libqxcb.a
        /usr/plugins/xcbglintegrations/libqxcb-glx-integration.a
        Qt5XcbQpa
        Qt5ServiceSupport
        Qt5ThemeSupport
        Qt5FontDatabaseSupport
        qtfreetype
        Qt5XkbCommonSupport
        Qt5GlxSupport
        Qt5EdidSupport
        Qt5DBus
        xcb-glx
        X11-xcb
        xcb
        Xext
        X11
        xcb-static
        xkbcommon-x11
        xkbcommon
        /usr/plugins/imageformats/libqgif.a
        /usr/plugins/imageformats/libqico.a
        /usr/plugins/imageformats/libqjpeg.a
        Qt5Widgets
        Qt5Gui
        qtlibpng
        qtharfbuzz
        Qt5Core
        m
        qtpcre2
        dl
        gthread-2.0
        glib-2.0
        GL
        GLX
        EGL
        pthread
    )
elseif (WIN32)
    target_link_libraries(speech_analysis
        -L/usr/mxe/usr/i686-w64-mingw32.static/lib
        -L/usr/mxe/usr/i686-w64-mingw32.static/qt5/lib
        /usr/mxe/usr/i686-w64-mingw32.static/qt5/plugins/platforms/libqminimal.a
        /usr/mxe/usr/i686-w64-mingw32.static/qt5/plugins/platforms/libqwindows.a 
        winspool
        wtsapi32
        Qt5EventDispatcherSupport
        Qt5FontDatabaseSupport
        dwrite
        d2d1
        Qt5ThemeSupport
        Qt5AccessibilitySupport
        Qt5WindowsUIAutomationSupport
        /usr/mxe/usr/i686-w64-mingw32.static/qt5/plugins/imageformats/libqgif.a
        /usr/mxe/usr/i686-w64-mingw32.static/qt5/plugins/imageformats/libqico.a
        /usr/mxe/usr/i686-w64-mingw32.static/qt5/plugins/imageformats/libqjpeg.a
        jpeg
        Qt5Widgets
        uxtheme
        dwmapi
        Qt5Gui
        d3d11
        dxgi
        dxguid
        harfbuzz
        cairo
        gobject-2.0
        fontconfig
        freetype
        m
        usp10
        msimg32
        pixman-1
        ffi
        expat
        bz2
        png16
        harfbuzz_too
        freetype_too
        glib-2.0
        shlwapi 
        pcre
        intl
        iconv
        gdi32
        comdlg32
        oleaut32
        imm32
        opengl32
        Qt5Core
        mpr
        userenv
        version
        z
        pcre2-16
        zstd
        netapi32
        ws2_32
        advapi32
        kernel32
        ole32
        shell32
        uuid
        user32
        winmm
        opengl32
        mingw32
        qtmain
        shell32    
    )
else()
    message("Target system not supported")
endif()
