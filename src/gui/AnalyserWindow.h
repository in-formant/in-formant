//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERWINDOW_H
#define SPEECH_ANALYSIS_ANALYSERWINDOW_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2_framerate.h>
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

class AnalyserWindow {
public:
    explicit AnalyserWindow() noexcept(false);
    ~AnalyserWindow() noexcept;

    void mainLoop();

private:
    void initTextures();
    void preRender();
    void render();
    void update();
    void handleKeyDown(const Uint8 * state, SDL_Scancode scanCode);
    void handleKeyUp(const Uint8 * state, SDL_Scancode scanCode);
    void handleMouse(int x, int y);

    // Graphics-related members
    SDL_Window * window;
    SDL_Renderer * renderer;
    TTF_Font * font;
    FPSmanager fpsManager;

    int targetWidth, targetHeight;
    SDL_Texture * pitchStrTex;
    std::vector<SDL_Texture *> formantStrTex;
    SDL_Texture * lpOrderStrTex;
    SDL_Texture * maxFreqStrTex;

    void renderGraph();

    // Other components.
    AudioCapture audioCapture;

    // Rendering parameters
    bool renderRaw, pauseScroll;
    int selectedFrame;

    // Analysis parameters
    double maximumFrequency;
    int lpOrder;

    // Data.
    Eigen::ArrayXd audioData;

    Formant::Frames rawFormantTrack;
    Formant::Frames formantTrack;
    std::deque<double> pitchTrack;

};


#endif //SPEECH_ANALYSIS_ANALYSERWINDOW_H
