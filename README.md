# Speech analysis software
### By Clo Yun-Hee Dufour.

===

## Description

This project is an attempt at making a software capable of analysing speech signals in real-time.
The main focus as of writing this description is on pitch detection and formant analysis.
Feel free to report any bugs, request new features, or critique existing ones in the Issues section.

## Usage

You can grab the latest release for your operating system in the [Releases](https://github.com/ichi-rika/speech-analysis/releases) page.

The graph represents frequencies (vertical axis) in function of time (horizontal axis).  
Audio is analysed in a succession of short frames (35-60 ms). One frame equals one "unit" on the horizontal axis.  

Formant analysis is performed on every frame. Moving the cursor at a particular point in time lets you read the formant frequencies in more detail.  

Pitch detection has the added benefit of functioning as a voice activity detector.  
