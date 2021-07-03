K:\Qt\6.1.2\msvc2019_64\bin\windeployqt --qmldir=src/qml build/in-formant.exe --release
cp build/external/libsamplerate/lsr.dll build

K:\Qt\6.1.2\msvc2019_64\bin\windeployqt --qmldir=src/qml build/in-formant.exe --release --dir=dist
cp build/external/libsamplerate/lsr.dll dist
cp build/in-formant.exe dist
cp build/fftw3.dll dist
cp build/freetype.dll dist
cp build/portaudio.dll dist
cp build/zlib1.dll dist
cp build/bz2.dll dist
cp build/libpng16.dll dist
cp build/brotlidec.dll dist
cp build/brotlicommon.dll dist
