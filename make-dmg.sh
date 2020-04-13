#!/bin/bash

IN_FILE="$(pwd)"/out.OSX
OUT_FILE=SpeechAnalysis

FILE="$(pwd)"/out.OSX.dmg

rm -f "$FILE"

dd if=/dev/zero of="$FILE" bs=1M count=7 status=progress

mkfs.hfsplus -v Install "$FILE"

sudo mkdir -pv /mnt/tmp
sudo mount -o loop "$FILE" /mnt/tmp
sudo cp -av "$(pwd)/osx-bundle/SpeechAnalysis.app" /mnt/tmp
sudo cp -av "$IN_FILE" /mnt/tmp/SpeechAnalysis.app/Contents/MacOS/"$OUT_FILE"
sudo umount /mnt/tmp

