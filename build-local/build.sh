#!/bin/bash
cd "$(readlink -f "${0%/*}")"

build_type=${1:-RelWithDebInfo}

function run_or_debug()
{
    if [ "x$build_type" = "xDebug" ]; then
        gdb build-local/in-formant -ex run
    else
        build-local/in-formant
    fi
}

cmake .. \
        -DCMAKE_BUILD_TYPE=$build_type \
        -DCMAKE_PREFIX_PATH=/opt/qt515 \
        -DTorch_DIR=$HOME/Builds/libtorch/share/cmake/Torch \
    && \
make -j$(nproc) && \
cd .. && \
run_or_debug


