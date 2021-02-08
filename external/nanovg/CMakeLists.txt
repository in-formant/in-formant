cmake_minimum_required(VERSION 3.5)
project(nanovg)

set(HEADERS
        src/fontstash.h
        src/nanovg.h
        src/nanovg_gl.h
        src/nanovg_gl_utils.h
        src/stb_image.h
        src/stb_truetype.h)

set(SOURCES
        src/nanovg.c)

add_library(nanovg STATIC ${HEADERS} ${SOURCES})

target_include_directories(nanovg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(SYSTEM_DARWIN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c -fobjc-arc")
    target_sources(nanovg PRIVATE src/nanovg_mtl.h src/nanovg_mtl.m)
    find_package(Qt5Gui REQUIRED)
    target_link_libraries(nanovg
        PUBLIC
        Qt5::GuiPrivate
        "-framework Foundation"
        "-framework Metal"
        "-framework QuartzCore"
        /osxcross/build/compiler-rt/compiler-rt/build/lib/darwin/libclang_rt.osx.a
    )
endif()
