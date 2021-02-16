#!/bin/bash

target=$1
build_type=$2

if [ -z "$target" ]; then
    target=linux
fi

src=$(pwd)
build=$(pwd)/build
dist=$(pwd)/dist

extra_args=

case $target in
    win32)
        tag=win32
        ;;
    win64)
        tag=win64
        ;;
    linux)
        tag=linux
        ;;
    macos)
        tag=macos
        ;;
    android)
        tag=android
        arch=${3:-x86}
        target=android-$arch
        ;;
esac

docker run $extra_args --rm -it -e TERM=xterm-256color -e CMAKE_BUILD_TYPE=$build_type -e target=$target -e ARCH=$arch -v $src:/src -v $build/$target:/build -v $dist:/dist clorika/sabuilder:$tag
