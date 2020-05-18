#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/linux
TARGET=$(pwd)/dist/linux

mkdir -p $BUILD_DIR
sudo rm -rf $TARGET/usr/bin
sudo rm -rf $TARGET/usr/lib
mkdir -p $TARGET

[[ -t 1 ]] && it_param=-it

docker rm -f extract-linux >/dev/null 2>&1

set -e
docker run --name extract-linux -v "$(pwd)":/src -v $BUILD_DIR:/build -v $TARGET:/target -e "CMAKE_BUILD_TYPE=$1" -e "VERBOSE=$VERBOSE" $it_param clorika/linux:latest
docker rm -f extract-linux

cd $(pwd)/dist && appimagetool linux speech_analysis-lin64.AppImage
