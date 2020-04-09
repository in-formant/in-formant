#!/bin/sh

BUILD_DIR=/tmp/speech-analysis/windows

mkdir -p $BUILD_DIR

docker container rm -f extract-win
docker container run --name extract-win -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" -a stdin -a stdout -a stderr clorika/windows:latest
docker container cp extract-win:/build/speech_analysis/src/main-build/speech_analysis.exe ./out.Windows.exe
docker container rm -f extract-win
