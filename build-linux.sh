#!/bin/sh

BUILD_DIR=/tmp/speech-analysis/linux

mkdir -p $BUILD_DIR

docker container rm -f extract-linux
docker container run --name extract-linux -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" clorika/linux:latest
docker container cp extract-linux:/build/speech_analysis/src/main-build/speech_analysis ./out.Linux
docker container rm -f extract-linux
