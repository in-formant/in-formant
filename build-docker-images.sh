#!/bin/bash

cd docker

for tag in linux win32 win64 macos android; do
    docker build -t clorika/sabuilder:$tag . -f Dockerfile.$tag
    docker tag clorika/sabuilder:$tag docker.io/clorika/sabuilder:$tag
    docker push clorika/sabuilder:$tag
done
