#!/bin/bash

target=$1
build_type=$2

if [ -z "$target" ]; then
    target=linux
fi

src=$(pwd)
build=$(pwd)/build
dist=$(pwd)/dist
: ${CCACHE_DIR:=$(pwd)/ccache}

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
    *)
        echo "unrecognized target $target!"
        echo "usage: $0 <target> [Release|RelWithDebInfo|Debug]"
        exit 1
        ;;
esac

mkdir -p $build/$target $dist
docker run $extra_args --rm -it -e TERM=xterm-256color -e CMAKE_BUILD_TYPE=$build_type -e ENABLE_TORCH=0 -e target=$target -e ARCH=$arch -v $src:/src -v $build/$target:/build -v $dist:/dist -v $CCACHE_DIR:/ccache clorika/sabuilder:$tag
