@echo off

docker build -f Dockerfile.Windows -t docker-windows:latest .

docker container create --name extract docker-windows:latest
docker container cp extract:/out ./out.Windows.exe
docker container rm -f extract