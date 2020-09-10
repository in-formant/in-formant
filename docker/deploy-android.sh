#!/bin/bash

set -e

tmp=/project

rm -rf $tmp
cp -r /src/dist-res/android-project $tmp

for target in android-arm android-arm64 android-x86 android-x86_64;
do
    . env-android.sh $target

    mkdir -p $tmp/app/libs/$android_abi

    cp -v /build/$target/libspeech-analysis.so $tmp/app/libs/$android_abi
    cp -v /build/$target/soxr/lib/libsoxr.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libhidapi.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libSDL2.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libfftw3.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libavcodec.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libavutil.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libswresample.so $tmp/app/libs/$android_abi
done

mkdir -p $tmp/app/src/main/assets

cp -v /src/Montserrat.otf $tmp/app/src/main/assets

if [ "x$1" == "xDebug" ]; then
    cd $tmp && gradle --no-daemon assembleDebug
    cp app/build/outputs/apk/debug/app-debug.apk /dist/speech-analysis.apk
else
    cd $tmp && gradle --no-daemon assembleRelease
    cp app/build/outputs/apk/release/app-release.apk /dist/speech-analysis.apk
fi

