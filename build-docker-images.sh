#!/bin/bash

cd docker

for tag in linux windows macos android emscripten; do
    docker build --rm -t clorika/sabuilder:$tag . -f Dockerfile.$tag
    docker tag clorika/sabuilder:$tag docker.io/clorika/sabuilder:$tag
    docker push clorika/sabuilder:$tag
done
