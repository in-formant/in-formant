#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/windows
TARGET=$(pwd)/dist/windows/speech_analysis

mkdir -p $BUILD_DIR
sudo rm -rf $TARGET
mkdir -p $TARGET

[[ -t 1 ]] && it_param=-it

docker rm -f extract-win >/dev/null 2>&1

set -e
docker run --name extract-win -v "$(pwd)":/src -v $BUILD_DIR:/build -v $TARGET:/target -e "CMAKE_BUILD_TYPE=$1" $it_param clorika/windows:latest
docker rm -f extract-win

cd $(pwd)/dist && zip -r speech_analysis-win32.zip $TARGET
