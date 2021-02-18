# InFormant™ - a real-time pitch and formant tracking software

## Table of Contents

* [Compiling from source](#how-to-compile-from-source-using-docker)
  * [Using Docker](#how-to-compile-from-source-using-docker)
  * [Locally](#how-to-compile-from-source-locally)

## How to compile from source (using Docker)

### Preparing the build environment

The build system uses Docker images in order to have a consistent build environment regardless of the actual machine that is being used for building.

You can pull prebuilt Docker images from the Docker Hub:

```sh
for arch in linux win32 win64 macos android ; do docker pull clorika/sabuilder:$arch ; done
```

However these are not always kept up to date with the various Dockerfiles included in the repository so you can always build them yourself:

```sh
cd docker
# Replace ARCH with one of [ linux, win32, win64, macos, android ]
docker build -t clorika/sabuilder:ARCH . -f Dockerfile.ARCH
```

**Note:** The Linux Dockerfile uses the Qt online installer so the build arguments `QT_EMAIL` and `QT_PW` must be provided. Do keep in mind that build arguments are kept in the final image. I will look for an alternative way to install Qt in the future.

### Building the project

Run the `build.sh` script from the project root.

The script takes two arguments:
- The first one is the target platform: must be one of `linux`, `win32`, `win64`, `macos`, `android`.
- The second one is the build configuration: must be one of `Release`, `RelWithDebInfo`, `Debug`.

There is an additional third argument for the `android` target to specify the target architecture.

Valid values are `arm`, `arm64`, `x86` (default if not given), and `x86_64`.

Multi-arch Android builds are already supported by the underlying build system but are not possible as of now.

**Note:** This script immediately creates a native package for the given target platform after a successful build. If you wish to build the project to contribute to it, you should use a local build environment. (see [How to compile from source locally](#how-to-compile-from-source-locally))

## How to compile from source locally

As of now, development is only really supported on Linux hosts, but is likely feasible on other Unix-like systems with some work.

Before anything, you must install the various dependencies. Specifics will vary depending on your distribution.

* Qt 5.15.2 (the Quick Controls 2 module and the Charts module are required in addition to the base package)
* FFTW3
* Eigen3
* libtorch built with C++11 ABI (from PyTorch)
* PortAudio v19 
* PulseAudio (optional, unstable due to race condition)
* ALSA (optional, causes inexplicably high CPU usage)

You will also need CMake and a compiler with C++17 support.

This should be all, you're ready to start building:

```sh
mkdir local
cd local
cmake ..
make -j$(nproc)
```

You may need to edit the CMake command line if some of those dependencies are installed to non-standard paths.

```sh
cmake .. -DCMAKE_PREFIX_PATH="/opt/Qt/5.15.2/gcc_64;/usr/local/libtorch"
```

You can also specify a build configuration between `Debug`, `RelWithDebInfo`, `MinSizeRel`, and `Release`.

```sh
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**Note:** the `Debug` build configuration enables ASan and UBSan by default.

## Prebuilt binaries

There are several prebuilt binaries included with each new release:

* an AppImage file, for Linux (simply set the executable flag and run it)
* two ZIP archives, for 32-bit and 64-bit Windows (the 32-bit will run on both editions of Windows)
* a DMG image, for macOS 10.14 Mojave or newer (only 64-bit PC architecture supported, no ARM binaries yet)
* an APK file, for Android 9 Pie or newer (**prefer [the Play Store version](https://play.google.com/store/apps/details?id=fr.cloyunhee.speechanalysis) for smaller downloads**)


