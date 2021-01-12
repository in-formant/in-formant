#!/bin/bash
cd "$(readlink -f "${0%/*}")"

build_type=${1:-RelWithDebInfo}

GLEW_SOURCE_FILE=glew.c cmake .. -DCMAKE_BUILD_TYPE=$build_type && make -j$(nproc) && cd .. && build-local/in-formant
