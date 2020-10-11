#!/bin/bash

build_type=$1
if [ "x$1" == "x" ]; then
    build_type=Release
fi

for target in linux win32 win64 macos android; do
    ./build.sh $target $build_type
done
