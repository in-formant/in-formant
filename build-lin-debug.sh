#!/bin/bash

export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

mkdir -p cmake-build-debug
cd cmake-build-debug

cmake -DCMAKE_BUILD_TYPE=Debug ..
make 
