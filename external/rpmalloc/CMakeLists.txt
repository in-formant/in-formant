cmake_minimum_required(VERSION 3.5)
project(rpmalloc)

set(CMAKE_AUTOMOC OFF)

add_library(rpmalloc INTERFACE)
target_include_directories(rpmalloc SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rpmalloc)

if(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(rpmalloc INTERFACE -DUSING_RPMALLOC=1)

    add_library(rpmalloc_shared SHARED rpmalloc/rpmalloc.c rpmalloc/rpcxx.cpp)
    target_compile_definitions(rpmalloc_shared PRIVATE -DBUILD_DYNAMIC_LINK -DENABLE_PRELOAD=1 -DENABLE_OVERRIDE=1)
    if(SYSTEM_LINUX)
        target_link_libraries(rpmalloc_shared PRIVATE ${CMAKE_DL_LIBS})
        target_compile_definitions(rpmalloc_shared PRIVATE -D_GNU_SOURCE)
    endif()
    target_include_directories(rpmalloc_shared SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rpmalloc)

    target_link_libraries(rpmalloc INTERFACE rpmalloc)
else()
    target_compile_definitions(rpmalloc INTERFACE -DUSING_RPMALLOC=0)
endif()

