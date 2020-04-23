#!/bin/bash

BUILD_DIR=/tmp/speech-analysis/android

mkdir -p $BUILD_DIR

[[ -t 1 ]] && it_param=-it

mkdir -p $HOME/.cache/speech_analysis_build/emscripten

docker rm -f extract-android >/dev/null 2>&1
docker run --name extract-android -v "$HOME/.cache/speech_analysis_build/android:/root/.gradle" -v "$(pwd)":/src -v $BUILD_DIR:/build -e "CMAKE_BUILD_TYPE=$1" $it_param clorika/android:latest
docker cp extract-android:/build/armeabi-v7a/speech_analysis/src/main-build/speech_analysis-armeabi-v7a/build/outputs/apk/release/speech_analysis-armeabi-v7a-release-signed.apk ./out.android.armeabi-v7a.apk
docker cp extract-android:/build/arm64-v8a/speech_analysis/src/main-build/speech_analysis-arm64-v8a/build/outputs/apk/release/speech_analysis-arm64-v8a-release-signed.apk ./out.android.arm64-v8a.apk
docker cp extract-android:/build/x86/speech_analysis/src/main-build/speech_analysis-x86/build/outputs/apk/release/speech_analysis-x86-release-signed.apk ./out.android.x86.apk
docker cp extract-android:/build/x86_64/speech_analysis/src/main-build/speech_analysis-x86_64/build/outputs/apk/release/speech_analysis-x86_64-release-signed.apk ./out.android.x86_64.apk

mkdir -p android-libspeech
docker cp extract-android:/build/armeabi-v7a/libspeech/src/libspeech-build/libspeech.so android-libspeech/libspeech.armeabi-v7a.so
docker cp extract-android:/build/arm64-v8a/libspeech/src/libspeech-build/libspeech.so android-libspeech/libspeech.arm64-v8a.so
docker cp extract-android:/build/x86/libspeech/src/libspeech-build/libspeech.so android-libspeech/libspeech.x86.so
docker cp extract-android:/build/x86_64/libspeech/src/libspeech-build/libspeech.so android-libspeech/libspeech.x86_64.so

docker rm -f extract-android
