#!/bin/bash

unset `env | \
    grep -vi '^EDITOR=\|^HOME=\|^LANG=\|OSXCROSS\|^PATH=\|^BUILD_WINDOWS=\|^EXE_WINDOWS=' | \
    grep -vi 'PKG_CONFIG\|PROXY\|^PS1=\|^TERM=' | \
    cut -d '=' -f1 | tr '\n' ' '`
export PATH=$OSXCROSS/target/bin:$PATH

export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

export CROSS=i386-apple-darwin17-

mkdir -p cmake-build-release-osx
cd cmake-build-release-osx

${CROSS}cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 4

