#!/bin/bash

set -e

tmp=/InFormant

mkdir -p $tmp

GCC_DLL_DIR=/usr/lib/gcc/$HOST/9.3-posix

touch $GCC_DLL_DIR/g++.exe
chmod +x $GCC_DLL_DIR/g++.exe

export PATH=/usr/$HOST/qt5/bin:$GCC_DLL_DIR:$PATH

/usr/$HOST/qt5/bin/windeployqt /build/in-formant.exe --dir $tmp --qmldir /usr/$HOST/qt5/qml

cp /src/Montserrat.otf $tmp/Montserrat.otf

cd /dist
zip -r InFormant-win32.zip $tmp
