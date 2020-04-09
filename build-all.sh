#!/bin/bash
echo -e "\n\e[33mBuilding...\n\e[39m"

echo -e "\e[92m * Building for Linux...\n\e[39m"
./build-lin.sh

echo -e "\n\e[92m * Building for Windows...\n\e[39m"
./build-win-mxe.sh

echo -e "\n\e[92m * Building for MacOS...\n\e[39m"
./build-osx.sh
