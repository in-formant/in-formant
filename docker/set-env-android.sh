#!/bin/bash

case $ARCH in
    arm)
        export ANDROID_ABI="armeabi-v7a with NEON"
        export HOST=armv7a-linux-androideabi
        export HOST2=arm-linux-androideabi
        ;;
    arm64)
        export ANDROID_ABI=arm64-v8a
        export HOST=aarch64-linux-android
        ;;
    x86)
        export ANDROID_ABI=x86
        export HOST=i686-linux-android
        ;;
    x86_64)
        export ANDROID_ABI=x86_64
        export HOST=x86_64-linux-android
        ;;
esac

if [ -z "$HOST2" ]; then
    export HOST2=$HOST
fi

export TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64

export INS_PREFIX=/android/$HOST

#export pkg_config_exe=$ins_prefix/bin/pkg-config
#export PKG_CONFIG_PATH=$ins_prefix/lib/pkgconfig:$ins_prefix/share/pkgconfig

#export cmake_flags="-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DCMAKE_INSTALL_PREFIX=$ins_prefix -DANDROID_NDK=$ANDROID_NDK_HOME -DANDROID_PLATFORM=$ANDROID_PLATFORM -DANDROID_ABI=$android_abi -DPKG_CONFIG_EXECUTABLE=$pkg_config_exe"

if [ -z "$2" ]; then
    export CC=$TOOLCHAIN/bin/${HOST}${ANDROID_API}-clang
    export CXX=$TOOLCHAIN/bin/${HOST}${ANDROID_API}-clang++

    export CMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake

    #export PKG_CONFIG_SYSROOT_DIR
    export PKG_CONFIG_LIBDIR=$INS_PREFIX/lib/pkgconfig:$INS_PREFIX/share/pkgconfig

    export configure_flags="--prefix=$INS_PREFIX --host=$HOST"
else
    export configure_flags="--prefix=$INS_PREFIX"
fi
