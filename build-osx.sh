#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/osx

mkdir -p $BUILD_DIR

[[ -t 1 ]] && it_param=-it

docker rm -f extract-osx >/dev/null 2>&1
docker run --name extract-osx -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" $it_param clorika/osx:latest
docker cp extract-osx:/build/speech_analysis/src/main-build/speech_analysis ./out.OSX
docker rm -f extract-osx
