//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERWINDOW_H
#define SPEECH_ANALYSIS_ANALYSERWINDOW_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <deque>
#include <vector>
#include "../audio/AudioCapture.h"
#include "../lib/Formant/Formant.h"
#include "Spectrogram.h"
#include "../Exceptions.h"

#define WINDOW_TITLE  "Speech analysis"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define WINDOW_FONT     "fonts/open-sans/OpenSans-Regular.ttf"
#define WINDOW_FONTSIZE 24

#define RENDER_RATE 120
#define RENDER_DELAY (1000 / RENDER_RATE)

#define UPDATE_RATE 120
#define UPDATE_DELAY (1000 / UPDATE_RATE)

class AnalyserWindow {
public:
    explicit AnalyserWindow() noexcept(false);
    ~AnalyserWindow() noexcept;

    void mainLoop();

private:
    void initTextures();
    void render();
    void update();
    void handleKey(const Uint8 * state);

    // Graphics-related members
    SDL_Window * window;
    SDL_Renderer * renderer;
    TTF_Font * font;

    int targetWidth, targetHeight;
    SDL_Texture * rawFormantTex;
    SDL_Texture * rawFormantTexCopy;
    SDL_Texture * formantTex;
    SDL_Texture * formantTexCopy;

    std::string pitchString;
    std::vector<std::string> formantString;

    // Other components.
    AudioCapture audioCapture;

    // Data.
    Eigen::ArrayXd audioData;

    bool renderRaw, pauseScroll;
    Formant::Frames rawFormantTrack;
    Formant::Frames formantTrack;
    std::deque<double> pitchTrack;

};


#endif //SPEECH_ANALYSIS_ANALYSERWINDOW_H
