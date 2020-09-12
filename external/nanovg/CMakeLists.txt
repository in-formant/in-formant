cmake_minimum_required(VERSION 3.5)
project(nanovg)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -D_CRT_SECURE_NO_WARNINGS -DFONS_USE_FREETYPE")

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

set(FT_DIR ${PROJECT_SOURCE_DIR}/../freetype)
target_link_libraries(nanovg freetype)
target_include_directories(nanovg PUBLIC ${FT_DIR}/include)

install(FILES $<TARGET_FILE:nanovg> DESTINATION lib)
install(FILES ${HEADERS}            DESTINATION include)
