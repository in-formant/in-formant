#!/bin/bash

target=$1

case $target in
    android-arm)
        android_abi=armeabi-v7a
        host=armv7a-linux-androideabi
        host2=arm-linux-androideabi
        ;;
    android-arm64)
        android_abi=arm64-v8a
        host=aarch64-linux-android
        ;;
    android-x86)
        android_abi=x86
        host=i686-linux-android
        ;;
    android-x86_64)
        android_abi=x86_64
        host=x86_64-linux-android
        ;;
esac

if [ -z "$host2" ]; then
    host2=$host
fi

toolchain=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64

export ins_prefix=/usr/$target

export pkg_config_exe=$ins_prefix/bin/pkg-config
export PKG_CONFIG_PATH=$ins_prefix/lib/pkgconfig:$ins_prefix/share/pkgconfig

export cmake_flags="-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DCMAKE_INSTALL_PREFIX=$ins_prefix -DANDROID_NDK=$ANDROID_NDK_HOME -DANDROID_PLATFORM=$ANDROID_PLATFORM -DANDROID_ABI=$android_abi -DPKG_CONFIG_EXECUTABLE=$pkg_config_exe"

if [ -z "$2" ]; then
    export AR=$toolchain/bin/$host2-ar
    export AS=$toolchain/bin/$host2-as
    export CC=$toolchain/bin/${host}${android_api}-clang
    export CXX=$toolchain/bin/${host}${android_api}-clang++
    export LD=$toolchain/bin/$host2-ld
    export RANLIB=$toolchain/bin/$host2-ranlib
    export STRIP=$toolchain/bin/$host2-stript

    export configure_flags="--prefix=$ins_prefix --host=$host"
    export host=$host
else
    export configure_flags="--prefix=$ins_prefix"
fi
