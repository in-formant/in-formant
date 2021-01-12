#!/bin/bash

set -e

tmp=/AppDir

qt_prefix=/opt/Qt5/5.12.10/gcc_64
qt_additional_libs=(
    $qt_prefix/lib/libQt5Quick.so.5
    $qt_prefix/lib/libQt5QuickControls2.so.5
    $qt_prefix/lib/libQt5QuickTemplates2.so.5
)
qt_platform_plugin=$qt_prefix/plugins/platforms/libqxcb.so
qt_qml_plugins=(
    $qt_prefix/qml/QtQuick.2
    $qt_prefix/qml/QtQuick
)

mkdir -p $tmp/usr/bin/platforms
mkdir -p $tmp/usr/bin
mkdir -p $tmp/usr/lib
mkdir -p $tmp/usr/share/applications
mkdir -p $tmp/usr/share/icons/hicolor/256x256

cp -v /build/in-formant $tmp/usr/bin/in-formant

for file in /build/in-formant $qt_platform_plugin; do
    for lib in $(ldd "$file" | grep -o '\W/[^ ]*'); do
        skip=false
        while IFS="" read -r p || [ -n "$p" ] ; do
            if [[ ( ! "$p" =~ ^# ) && ( ! -z "$p" ) && ( "$(basename "$lib")" == "$p" ) ]]; then
                skip=true
            fi
        done < "/src/dist-res/appimage-excludelist"

        libname=$(basename "$lib")
        if $skip || [[ -f "$tmp/usr/lib/$libname" ]]; then
            :
            #echo Skipping "$lib"
        else
            cp -v $lib $tmp/usr/lib
        fi
    done
done

cp -v $qt_platform_plugin $tmp/usr/bin/platforms
cp -v ${qt_additional_libs[@]} $tmp/usr/lib
cp -rv ${qt_qml_plugins[@]} $tmp/usr/bin
cp -v /src/dist-res/in-formant.desktop $tmp/usr/share/applications
cp -v /src/dist-res/in-formant.png $tmp/usr/share/icons/hicolor/256x256
cp -v /src/dist-res/AppRun $tmp/AppRun
ln -sfv usr/share/applications/in-formant.desktop $tmp/in-formant.desktop
ln -sfv usr/share/icons/hicolor/256x256/in-formant.png $tmp/in-formant.png
cp -v /src/Montserrat.otf $tmp/usr/Montserrat.otf

cd /dist && appimagetool --appimage-extract-and-run /AppDir in-formant.AppImage

