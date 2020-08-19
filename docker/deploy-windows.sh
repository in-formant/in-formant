#!/bin/bash

set -e

tmp=/speech-analysis

mkdir -p $tmp

cp -v /build/speech-analysis.exe $tmp
cp -rv /build/shaders $tmp
$MXE/tools/copydlldeps.sh \
        --infile /build/speech-analysis.exe \
        --destdir $tmp \
        --recursivesrcdir $MXE/usr/${cross::-1} \
        --recursivesrcdir /build/soxr \
        --objdump $MXE/usr/bin/${cross}objdump \
        --copy
cp -v /src/Montserrat.ttf $tmp

cd /dist
zip -r speech-analysis-$target.zip $tmp
