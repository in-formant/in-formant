#!/bin/bash

set -e

tmp=/AppDir

mkdir -p $tmp/usr

cd /dist

export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE=/opt/Qt/6.1.2/gcc_64/bin/qmake
export QML_SOURCES_PATHS=/src/src

if [ "x$ENABLE_TORCH" = "x1" ]; then
	export OUTPUT=InFormant-DF-$version-Linux-x86_64.AppImage
else
	export OUTPUT=InFormant-$version-Linux-x86_64.AppImage
fi

linuxdeploy \
    --appdir $tmp \
    --plugin qt \
    --executable /build/in-formant \
    --desktop-file /src/dist-res/in-formant.desktop \
    --icon-file /src/dist-res/in-formant.png \
    --output appimage
