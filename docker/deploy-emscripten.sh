#!/bin/bash

set -e

tmp=/speech-analysis-html5

rm -rf $tmp
cp -r /src/dist-res/html5-project $tmp

cp -v /build/speech-analysis.data $tmp
cp -v /build/speech-analysis.js $tmp
cp -v /build/speech-analysis.wasm $tmp
cp -v /build/speech-analysis.worker.js $tmp

if [ -f /build/speech-analysis.wasm.map ]; then
    cp -v /build/speech-analysis.wasm.map $tmp
fi

cd /dist
tar cvf speech-analysis-html5.tgz $tmp
