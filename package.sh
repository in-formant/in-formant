#!/bin/bash

export BUILD_SUFFIX=speech_analysis/src/main-build

export BUILD_LINUX=$(pwd)/cmake-build-release/$BUILD_SUFFIX
export BUILD_WINDOWS=$(pwd)/cmake-build-release-win/$BUILD_SUFFIX

export EXE_LINUX=$BUILD_LINUX/speech_analysis
export EXE_WINDOWS=$BUILD_WINDOWS/speech_analysis.exe

echo -e "\n\e[33mBuilding...\n\e[39m"

echo -e "\e[92m * Building for Linux...\n\e[39m"
./build-lin.sh

echo -e "\n\e[92m * Building for Windows...\n\e[39m"
./build-win-mxe.sh

echo -e "\n\e[33mPackaging...\n\e[39m"

rm -rf pkg
mkdir pkg
cd pkg

echo -e "\e[92m * Packaging for Linux...\n\e[39m"

rm -rf speech-analysis
mkdir -p speech-analysis
cp $EXE_LINUX speech-analysis/
zip speech-analysis-linux.zip -r speech-analysis

echo -e "\n\e[92m * Packaging for Windows...\n\e[39m"

rm -rf speech-analysis
mkdir -p speech-analysis
cp $EXE_WINDOWS speech-analysis/
cp -r $BUILD_WINDOWS/*.dll speech-analysis/
zip speech-analysis-windows.zip -r speech-analysis

rm -rf speech-analysis

echo -e "\n\e[96mFinished packaging!\n\n\e[39m"
