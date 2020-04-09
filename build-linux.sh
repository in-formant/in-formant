#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/linux

mkdir -p $BUILD_DIR

[[ -t 1 ]] && it_param=-it

docker rm -f extract-linux >/dev/null 2>&1
docker run --name extract-linux -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" $it_param clorika/linux:latest
docker cp extract-linux:/build/speech_analysis/src/main-build/speech_analysis ./out.Linux
docker rm -f extract-linux
