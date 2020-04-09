#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/windows

mkdir -p $BUILD_DIR

[[ -t 1 ]] && it_param=-it

docker rm -f extract-win >/dev/null 2>&1
docker run --name extract-win -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" $it_param clorika/windows:latest
docker cp extract-win:/build/speech_analysis/src/main-build/speech_analysis.exe ./out.Windows.exe
docker rm -f extract-win
