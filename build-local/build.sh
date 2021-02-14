#!/bin/bash
cd "$(readlink -f "${0%/*}")"

build_type=${1:-RelWithDebInfo}

function run_or_debug()
{
    if [ "x$build_type" = "xDebug" ] || [ "x$1" = "xd" ]; then
        gdb build-local/in-formant -ex run
    else
        build-local/in-formant
    fi
}

export CMAKE_PREFIX_PATH="/opt/Qt/5.15.2/gcc_64;/usr/local/libtorch"
export CC=clang-8
export CXX=clang++-8

if [ "x$build_type" = "xp" ]; then
    cmake .. \
            -DWITH_PROFILER=ON \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH \
        && \
    make -j$(nproc) && \
    cd .. && \
    env CPUPROFILE=build-local/in-formant.prof build-local/in-formant
    google-pprof --web build-local/in-formant build-local/in-formant.prof
else
    cmake .. \
            -DCMAKE_BUILD_TYPE=$build_type \
            -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH \
        && \
    make -j$(nproc) && \
    cd .. && \
    run_or_debug $2
fi
