#!/bin/bash

set -e

tmp=/InFormant.app

mkdir -p $tmp/Contents/MacOS
mkdir -p $tmp/Contents/Resources

cp /src/dist-res/Info.plist $tmp/Contents
cp /build/in-formant $tmp/Contents/MacOS/InFormant
cp /src/resources/icons/in-formant.icns $tmp/Contents/Resources

# The qsvg imageformat plugin is not exported by macdeployqt for some reason.
mkdir -p $tmp/Contents/PlugIns/imageformats
#cp /osxcross/target/$HOST/qt5/plugins/imageformats/libqsvg.dylib $tmp/Contents/PlugIns/imageformats

sed -i 's/@VERSION@/'"$version"'/g' $tmp/Contents/Info.plist 

export OSXCROSS_TARGET_DIR=/osxcross/target

/osxcross/target/bin/macdeployqt $tmp -dmg -qmldir=/src/src/qml

srcdmg=${tmp/.app/.dmg}

if [ "x$ENABLE_TORCH" = "x1" ]; then
	distdmg=/dist/InFormant-DF-$version-macOS-x86_64.dmg
else
	distdmg=/dist/InFormant-$version-macOS-x86_64.dmg
fi

hfsplus $srcdmg chmod 0755 /InFormant.app/Contents/MacOS/InFormant 

dmg dmg $srcdmg $distdmg
