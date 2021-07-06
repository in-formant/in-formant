#!/bin/bash

set -e

tmp=/InFormant-$version

mkdir -p $tmp

GCC_DLL_DIR=/usr/lib/gcc/$HOST/9.3-posix

touch $GCC_DLL_DIR/g++.exe
chmod +x $GCC_DLL_DIR/g++.exe

export PATH=/opt/Qt/6.1.2/mingw81_64/bin:$GCC_DLL_DIR:$PATH
export LD_LIBRARY_PATH=/opt/Qt/6.1.2/gcc_64/lib:$LD_LIBRARY_PATH

/opt/Qt/6.1.2/mingw81_64/bin/windeployqt /build/in-formant.exe --dir $tmp --qmldir /src/src/qml

if [ "x$ENABLE_TORCH" = "x1" ]; then
	name=InFormant-DF-$version
else
	name=InFormant-$version
fi

cp /build/in-formant.exe $tmp/$name.exe

cp /build/external/libsamplerate/liblsr.dll \
    /build/external/freetype/libfreetype.dll \
    /usr/lib/gcc/$HOST/9.3-posix/libgomp-1.dll \
    /usr/lib/gcc/$HOST/9.3-posix/libgfortran-5.dll \
    /usr/lib/gcc/$HOST/9.3-posix/libquadmath-0.dll \
    /usr/$HOST/bin/libwinpthread-1.dll \
    /usr/$HOST/bin/libfftw3-3.dll \
    /usr/$HOST/lib/libportaudio-2.dll \
    $tmp

if [ "x$ENABLE_TORCH" = "x1" ]; then
    cp /usr/libtorch/bin/libopenblas.dll \
        /usr/libtorch/lib/libc10.dll \
        /usr/libtorch/lib/libtorch_cpu.dll \
        $tmp
fi

cd /dist
zip -9 -r $name-$DIST_SUFFIX.zip $tmp
