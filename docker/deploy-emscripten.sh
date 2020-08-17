#!/bin/bash

set -e

tmp=/project

rm -rf $tmp
cp -r /src/dist-res/html5-project $tmp

for target in android-arm android-arm64 android-x86 android-x86_64;
do
    . env-android.sh $target

    mkdir -p $tmp/app/libs/$android_abi

    cp -v /build/$target/libpitch-tracker.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libhidapi.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libSDL2.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libSDL2_ttf.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libSDL2_gfx.so $tmp/app/libs/$android_abi
    cp -vL /usr/$target/lib/libfftw3.so $tmp/app/libs/$android_abi
done

cp -v /src/Montserrat.ttf $tmp/app/src/main/assets

cd $tmp && gradle --no-daemon assembleRelease
cp app/build/outputs/apk/release/app-release.apk /dist/pitch-tracker.apk

