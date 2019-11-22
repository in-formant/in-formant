# Speech analysis software
### By Clo Yun-Hee Dufour.

(Unfinished README)

===

## Description

This project is an attempt at making a software capable of analysing speech signals in real-time.
The main focus as of writing this description is on pitch detection and formant analysis.
Feel free to report any bugs, request new features, or critique existing ones in the Issues section.

## Usage

You can grab the latest release for your operating system in the [Releases](https://github.com/ichi-rika/speech-analysis/releases) page.  
If you are on Windows, the release should ship with its dependencies.  
If you are on GNU/Linux, jump to the [Dependencies](#dependencies) section.  

After unzipping the release, run the `speech_analysis` executable.  

The graph represents frequencies (vertical axis) in function of time (horizontal axis).  
Audio is analysed in a succession of short frames (15-60 ms). One frame equals one "unit" on the horizontal axis.  

Formant analysis is performed on every frame. Moving the cursor at a particular point in time lets you read the formant frequencies in more detail.  

Pitch detection has the added benefit of functioning as a voice activity detector.  

Unvoiced frames will display formant tracks in dim red.  
Voiced frames will display pitch as a light blue marker, and mark formant tracks with color-coded circles.  

Pressing `P` lets you pause the analysis.  
Pressing `S` lets you toggle between linear scale and logarithmic scale.  
~~Holding `R` lets you view the raw formant data.~~ (Has no effect as of now)  
Holding `L` and pressing `Up` or `Down` lets you adjust the order of the linear predictive coding analysis.  
Holding `F` and pressing `Up` or `Down` lets you adjust the maximum frequency of any expected formant.  

## Dependencies

SDL2  
SDL2_gfx  
SDL2_ttf  
PortAudio19  
Eigen-unstable  
FFTW3  
