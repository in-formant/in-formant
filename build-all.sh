#!/bin/bash

echo -e "\n\e[33mBuilding...\n\e[39m"

mkdir -p /tmp/speech-analysis

trap "kill \$p1; kill \$p2; kill \$p3; exit" SIGINT

{
    echo -e "\e[36m * Building for Linux...\n\e[39m"
    logfile=/tmp/speech-analysis/linux.log
    rm -f $logfile
    ./build-linux.sh $1 1>>$logfile 2> >(tee -a $logfile >&2)
    echo -e "\e[92m Linux build finished.\e[39m"
} &
p1=$!

{
    echo -e "\e[36m * Building for Windows...\n\e[39m"
    logfile=/tmp/speech-analysis/windows.log
    rm -f $logfile
    ./build-windows.sh $1 1>>$logfile 2> >(tee -a $logfile >&2)
    echo -e "\e[92m Windows build finished.\e[39m"
} &
p2=$!

{
    echo -e "\e[36m * Building for MacOS...\n\e[39m"
    logfile=/tmp/speech-analysis/osx.log
    rm -f $logfile
    ./build-osx.sh $1 1>>$logfile 2> >(tee -a $logfile >&2)
    echo -e "\e[92m OS X build first pass finished.\e[39m"
    sleep 1
    ./build-osx.sh $1 1>>$logfile 2> >(tee -a $logfile >&2)
    echo -e "\e[92m OS X build second pass finished.\e[39m"
} &
p3=$!

wait
wait
wait

echo -e "\n\e[33mCompressing binaries...\n\e[39m"

{
    upx --lzma -qq out.Linux
    echo -e "\e[92m Linux binary compressed.\e[39m"
} &
p1=$!

{
    upx --lzma -qq out.Windows.exe
    echo -e "\e[92m Windows binary compressed.\e[39m"
} &
p2=$!

{
    upx --lzma -qq out.OSX
    echo -e "\e[92m OS X binary compressed.\e[39m"
} &
p3=$!

wait 
wait
wait


