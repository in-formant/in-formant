#!/bin/sh

BUILD_DIR=/tmp/speech-analysis/osx

mkdir -p $BUILD_DIR

docker container rm -f extract-osx
docker container run --name extract-osx -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" -a stdin -a stdout -a stderr clorika/osx:latest
docker container cp extract-osx:/build/speech_analysis/src/main-build/speech_analysis ./out.OSX
docker container rm -f extract-osx
