cmake_minimum_required(VERSION 3.5)
project(nanovg)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -D_CRT_SECURE_NO_WARNINGS")

set(HEADERS
        src/fontstash.h
        src/nanovg.h
        src/nanovg_gl.h
        src/nanovg_gl_utils.h
        src/stb_image.h
        src/stb_truetype.h)

set(SOURCES
        src/nanovg.c)

#[[ Future support for Metal render
if(APPLE)
    list(APPEND SOURCES
        src/nanovg_mtl.m
        src/nanovg_mtl_shaders.metal)
endif()
]]

add_library(nanovg STATIC ${HEADERS} ${SOURCES})

install(FILES $<TARGET_FILE:nanovg> DESTINATION lib)
install(FILES ${HEADERS}            DESTINATION include)
