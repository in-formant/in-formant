#!/bin/bash
cd "${0%/*}"
GLEW_SOURCE_FILE=glew.c cmake .. && make -j$(nproc) && ./in-formant
