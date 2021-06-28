#!/bin/bash

set -e

tmp=/InFormant-$version

mkdir -p $tmp

GCC_DLL_DIR=/usr/lib/gcc/$HOST/9.3-posix

touch $GCC_DLL_DIR/g++.exe
chmod +x $GCC_DLL_DIR/g++.exe

export PATH=/usr/$HOST/qt5/bin:$GCC_DLL_DIR:$PATH

/usr/$HOST/qt5/bin/windeployqt /build/in-formant.exe --dir $tmp --qmldir /src/src/qml

cp /build/in-formant.exe $tmp/InFormant-$version.exe
cp /build/external/soxr/bin/libsoxr.dll $tmp
cp /build/external/freetype/libfreetype.dll $tmp

cp /usr/lib/gcc/$HOST/9.3-posix/libgomp-1.dll \
    /usr/lib/gcc/$HOST/9.3-posix/libgfortran-5.dll \
    /usr/lib/gcc/$HOST/9.3-posix/libquadmath-0.dll \
    /usr/$HOST/bin/libfftw3-3.dll \
    /usr/$HOST/bin/libwinpthread-1.dll \
    /usr/$HOST/bin/zlib1.dll \
    /usr/$HOST/lib/libportaudio-2.dll \
    /usr/libtorch/bin/libopenblas.dll \
    /usr/libtorch/lib/libc10.dll \
    /usr/libtorch/lib/libtorch_cpu.dll \
    $tmp



cd /dist
zip -9 -r InFormant-$version-$DIST_SUFFIX.zip $tmp
