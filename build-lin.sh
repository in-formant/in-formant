#!/bin/bash

export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

mkdir -p cmake-build-release
cd cmake-build-release

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-static -DCMAKE_EXE_LINKER_FLAGS=-static ..
make -j 4 -k
