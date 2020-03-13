#!/bin/sh

BUILD_DIR=/tmp/speech-analysis/osx

mkdir -p $BUILD_DIR

docker pull clorika/osx:latest
docker container run --name extract -v "$(pwd)":/src -v $BUILD_DIR:/build clorika/osx:latest
docker container cp extract:/build/speech_analysis/src/main-build/speech_analysis.app ./out.OSX.app
docker container rm -f extract
