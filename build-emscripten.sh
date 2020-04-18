#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/emscripten

mkdir -p $BUILD_DIR

[[ -t 1 ]] && it_param=-it

mkdir -p $HOME/.cache/speech_analysis_build/emscripten

docker rm -f extract-em >/dev/null 2>&1
docker run --name extract-em -v "$(pwd)":/src -v "$HOME/.cache/speech_analysis_build/emscripten:/root/.emscripten_cache" -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" $it_param clorika/emscripten:latest

docker cp extract-em:/build/speech_analysis/src/main-build/speech_analysis.js           web-bundle/
docker cp extract-em:/build/speech_analysis/src/main-build/speech_analysis.worker.js    web-bundle/
docker cp extract-em:/build/speech_analysis/src/main-build/speech_analysis.wasm         web-bundle/

docker rm -f extract-em
