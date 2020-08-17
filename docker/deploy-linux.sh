#!/bin/bash

set -e

tmp=/AppDir

mkdir -p $tmp/usr/bin
mkdir -p $tmp/usr/lib
mkdir -p $tmp/usr/share/applications
mkdir -p $tmp/usr/share/icons/hicolor/256x256

cp -v /build/speech-analysis $tmp/usr/bin/speech-analysis
cp -rv /build/shaders $tmp/usr/shaders

for lib in $(ldd /build/speech-analysis | grep -o '\W/[^ ]*'); do
    skip=false
    while IFS="" read -r p || [ -n "$p" ] ; do
        if [[ ( ! "$p" =~ ^# ) && ( ! -z "$p" ) && ( "$(basename "$lib")" == "$p" ) ]]; then
            skip=true
        fi
    done < "/src/dist-res/appimage-excludelist"

    if $skip; then
        :
        #echo Skipping "$lib"
    else
        cp -v $lib $tmp/usr/lib
    fi
done

cp -v /src/dist-res/speech-analysis.desktop $tmp/usr/share/applications
cp -v /src/dist-res/speech-analysis.png $tmp/usr/share/icons/hicolor/256x256
cp -v /src/dist-res/AppRun $tmp/AppRun
ln -sfv usr/share/applications/speech-analysis.desktop $tmp/speech-analysis.desktop
ln -sfv usr/share/icons/hicolor/256x256/speech-analysis.png $tmp/speech-analysis.png
cp -v /src/Montserrat.ttf $tmp/usr/Montserrat.ttf

cd /dist && appimagetool --appimage-extract-and-run /AppDir speech-analysis.AppImage

