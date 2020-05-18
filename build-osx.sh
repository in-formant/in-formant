#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/osx
TARGET=$(pwd)/dist/osx

mkdir -p $BUILD_DIR
sudo rm -rf $TARGET/Contents/MacOS
sudo rm -rf $TARGET/Contents/Frameworks
mkdir -p $TARGET

[[ -t 1 ]] && it_param=-it

docker rm -f extract-osx >/dev/null 2>&1

set -e
docker run --name extract-osx -v "$(pwd)":/src -v $BUILD_DIR:/build -v $TARGET:/target -e "CMAKE_BUILD_TYPE=$1" -e "VERBOSE=$VERBOSE" $it_param clorika/osx:latest
docker rm -f extract-osx

dirsize=`du -b -d 0 "$TARGET" | cut -f1`
dirsize=$(($dirsize / 1024 / 1024))

filename=speech_analysis-mac64.dmg

cd $(pwd)/dist
dd if=/dev/zero of="$filename" bs=1280K count="$dirsize" status=progress

mkfs.hfsplus -v "Install Speech Analysis" "$filename"

mntpt=/tmp/speech-analysis/dmg-mount
mkdir -p "$mntpt"
sudo mount -o loop "$filename" "$mntpt"
sudo mkdir -p "$mntpt/SpeechAnalysis.app"
sudo cp -r "$TARGET"/* "$mntpt/SpeechAnalysis.app"
sudo umount "$mntpt"
