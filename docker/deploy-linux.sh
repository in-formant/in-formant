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
    $qt_prefix/qml/QtQuick
    $qt_prefix/qml/QtQuick.2
)

mkdir -p $tmp/usr/bin/qml
mkdir -p $tmp/usr/bin/platforms
mkdir -p $tmp/usr/bin
mkdir -p $tmp/usr/lib
mkdir -p $tmp/usr/share/applications
mkdir -p $tmp/usr/share/icons/hicolor/256x256

cp /build/in-formant $tmp/usr/bin/in-formant

function copy_libs()
{
    lib=$1
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
        cp $lib $tmp/usr/lib
    fi
}

for file in /build/in-formant $qt_platform_plugin; do
    for lib in $(ldd "$file" | grep -o '\W/[^ ]*'); do
        copy_libs $lib
    done
done

for lib in ${qt_additional_libs[@]}; do
    copy_libs $lib 
done

cp $qt_platform_plugin $tmp/usr/bin/platforms
cp -r ${qt_qml_plugins[@]} $tmp/usr/bin/qml
cp /src/dist-res/in-formant.desktop $tmp/usr/share/applications
cp /src/dist-res/in-formant.png $tmp/usr/share/icons/hicolor/256x256
cp /src/dist-res/AppRun $tmp/AppRun
ln -sf usr/share/applications/in-formant.desktop $tmp/in-formant.desktop
ln -sf usr/share/icons/hicolor/256x256/in-formant.png $tmp/in-formant.png
cp /src/Montserrat.otf $tmp/usr/Montserrat.otf

cd /dist && appimagetool --appimage-extract-and-run /AppDir in-formant.AppImage

