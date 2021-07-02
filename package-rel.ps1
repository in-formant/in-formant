K:\Qt\6.1.2\msvc2019_64\bin\windeployqt --qmldir=src/qml build/in-formant.exe --dir=dist --release
cp build/in-formant.exe dist
cp build/fftw3.dll dist
cp build/freetype.dll dist
cp build/portaudio.dll dist
cp build/external/soxr/bin/soxr.dll dist
cp build/zlib1.dll dist
cp build/bz2.dll dist
cp build/libpng16.dll dist
cp build/brotlidec.dll dist
cp build/brotlicommon.dll dist