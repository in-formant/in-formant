#!/bin/sh

docker build -f Dockerfile.Linux -t docker-linux:latest .

docker container create --name extract docker-linux:latest
docker container cp extract:/out ./out.Linux
docker container rm -f extract