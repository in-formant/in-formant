#!/bin/bash

unset `env | \
    grep -vi '^EDITOR=\|^HOME=\|^LANG=\|MXE\|^PATH=' | \
    grep -vi 'PKG_CONFIG\|PROXY\|^PS1=\|^TERM=' | \
    cut -d '=' -f1 | tr '\n' ' '`
export PATH=/opt/mxe/usr/bin:$PATH

export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

mkdir -p cmake-build-release-win
cd cmake-build-release-win

i686-w64-mingw32.static-cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 4 -k
