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
    win32|win64)
        tag=windows
        ;;
    linux)
        tag=linux
        ;;
    macos)
        tag=macos
        extra_args="--privileged=true"
        ;;
    android)
        tag=android
        if [ "$build_type" == "Debug" ]; then
            extra_args="-e debug=Debug"
        fi
        ;;
    emscripten)
        tag=emscripten
        ;;
esac

docker run $extra_args --rm -it -e TERM=xterm-256color -e CMAKE_BUILD_TYPE=$build_type -e target=$target -v $src:/src -v $build/$target:/build -v $dist:/dist clorika/sabuilder:$tag
