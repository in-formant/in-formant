#!/bin/bash

set -e

tmp=/InFormant.app

mkdir -p $tmp/Contents/MacOS

cp /src/dist-res/Info.plist $tmp/Contents
cp /build/in-formant $tmp/Contents/MacOS/InFormant

sed -i 's/@VERSION@/'"$version"'/g' $tmp/Contents/Info.plist 

export OSXCROSS_TARGET_DIR=/osxcross/target

/osxcross/target/$HOST/qt5/bin/macdeployqt $tmp -dmg -qmldir=/src/resources

srcdmg=${tmp/.app/.dmg}
distdmg=/dist/InFormant-$version-macOS-x86_64.dmg

hfsplus $srcdmg chmod 0755 /InFormant.app/Contents/MacOS/InFormant 

rm -rf /dist/InFormant.app
cp -r /InFormant.app /dist

rm -f $distdmg
#dmg dmg $srcdmg $distdmg
