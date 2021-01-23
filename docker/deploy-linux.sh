#!/bin/bash

set -e

tmp=/AppDir

mkdir -p $tmp/usr
cp /src/Montserrat.otf $tmp/usr/Montserrat.otf

cd /dist

export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE=/opt/Qt5/5.12.10/gcc_64/bin/qmake
export QML_SOURCES_PATHS=/src/resources

linuxdeploy \
    --appdir $tmp \
    --plugin qt \
    --executable /build/in-formant \
    --desktop-file /src/dist-res/in-formant.desktop \
    --icon-file /src/dist-res/in-formant.png \
    --output appimage

