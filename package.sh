#!/bin/bash

export MXE=/mnt/Fast/mxe

./build-lin.sh
./build-win-mxe.sh

rm -rf pkg
mkdir pkg
cd pkg

rm -rf speech-analysis
mkdir -p speech-analysis
cp ../cmake-build-release/speech_analysis speech-analysis/
cp -r ../fonts speech-analysis/
zip speech-analysis-lin.zip -r speech-analysis

rm -rf speech-analysis
mkdir -p speech-analysis
cp ../cmake-build-release-win/speech_analysis.exe speech-analysis/
cp ../cmake-build-release-win/*.dll speech-analysis/
cp -r ../fonts speech-analysis/
zip speech-analysis-win.zip -r speech-analysis
