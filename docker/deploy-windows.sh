#!/bin/bash

set -e

tmp=$HOME/InFormant-$version

mkdir -p $tmp

if [ "x$CMAKE_BUILD_TYPE" = "xDebug" ]; then
    deploy_extra_args=--debug
else
    deploy_extra_args=--release
fi

vcwine /opt/Qt/6.1.2/msvc2019_64/bin/windeployqt.exe /build/in-formant.exe --dir $tmp --qmldir /src/src/qml $deploy_extra_args

if [ "x$ENABLE_TORCH" = "x1" ]; then
	name=InFormant-DF-$version
else
	name=InFormant-$version
fi

cp /build/in-formant.exe $tmp/$name.exe

cp /build/external/libsamplerate/lsr.dll \
    /opt/win/drive_c/usr/bin/fftw3.dll \
    /opt/win/drive_c/usr/bin/portaudio_x64.dll \
    $tmp

if [ "x$CMAKE_BUILD_TYPE" = "xDebug" ]; then
    cp /build/external/freetype/freetyped.dll $tmp
else
    cp /build/external/freetype/freetype.dll  $tmp
fi

if [ "x$ENABLE_TORCH" = "x1" ]; then
    cp /usr/libtorch/bin/libopenblas.dll \
        /usr/libtorch/lib/libc10.dll \
        /usr/libtorch/lib/libtorch_cpu.dll \
        $tmp
fi

cd $HOME
zip -9 -r /dist/$name-$DIST_SUFFIX.zip $(basename $tmp)
