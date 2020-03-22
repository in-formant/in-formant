cmake_minimum_required(VERSION 3.0)
project(main)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-fPIC -ftree-vectorize")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Ofast")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    main.cpp
    Exceptions.cpp
    Exceptions.h
    log/simpleQtLogger.cpp
    log/simpleQtLogger.h
    analysis/Analyser_mainLoop.cpp
    analysis/Analyser.cpp
    analysis/Analyser.h
    analysis/parts/formants.cpp
    analysis/parts/lpc.cpp
    analysis/parts/smooth.cpp
    analysis/parts/pitch.cpp
    analysis/parts/preprocess.cpp
    analysis/parts/resample.cpp
    analysis/parts/spectrum.cpp
    analysis/parts/track.cpp
    audio/AudioCapture.cpp
    audio/AudioCapture_callbacks.cpp
    audio/AudioDevices.cpp
    audio/AudioDevices.h
    audio/RingBuffer.cpp
    audio/RingBuffer.h
    audio/miniaudio.h
    audio/implementation.cpp
    gui/AnalyserCanvas.cpp
    gui/AnalyserCanvas.h
    gui/MainWindow.cpp
    gui/MainWindow.h)

set(QT_SOURCES
    qt_plugin.cpp
    ../resources.qrc)

if(NOT APPLE)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
else()
    set(QT_PREFIX /usr/osxcross/target/macports/pkgs/opt/local/libexec/qt5)
    set(Qt5Widgets_INCLUDE_DIRS
        ${QT_PREFIX}/include
        ${QT_PREFIX}/include/QtCore
        ${QT_PREFIX}/include/QtGui
        ${QT_PREFIX}/include/QtWidgets
    )
    set(Qt5Widgets_LIBRARIES
        Qt5Core
        Qt5Gui
        Qt5Widgets
    )
endif()

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(FFTW3 REQUIRED)

include_directories(
	${libspeech_INCLUDE_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${EIGEN_INCLUDE_DIRS}
    ${FFTW3_INCLUDE_DIRS}
)

add_executable(speech_analysis WIN32 ${SOURCES} ${QT_SOURCES})

target_link_libraries(speech_analysis
    ${libspeech_LIBRARY}
    Eigen3::Eigen
    FFTW3::fftw3
)

if(APPLE)
    target_link_libraries(speech_analysis
        -framework Qt5Core
        ${Qt5Core_LIBRARIES}
        ${Qt5Gui_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
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
