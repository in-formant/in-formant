#!/bin/bash

if [ ! -f shaderc.tgz ]; then
    x=$(which wget && which awk)
    if [ $? != 0 ]; then
        echo "Could not find wget or gawk"
    else
        url=$(wget https://storage.googleapis.com/shaderc/badges/build_link_linux_clang_release.html -qO- | \
            awk -v \
                'RS=http-equiv="[rR]efresh" *content="[0-9 ;]*[uU][rR][lL]=' \
                -F '"' \
                '/^http/{print $1;exit}')
        if [ $? != 0 ]; then
            echo "Could not find download URL for shaderc linux_clang release"
        else
            wget "$url" -O shaderc.tgz
        fi
    fi
else
    echo "Found existing shaderc.tgz file"
fi

echo "Extracting shaderc.tgz"
rm -rf shaderc/
tar xvf shaderc.tgz --transform s/install/shaderc/ --show-transformed
