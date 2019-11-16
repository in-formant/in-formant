//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_ANALYSERWINDOW_H
#define SPEECH_ANALYSIS_ANALYSERWINDOW_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2_framerate.h>
#include <vector>
#include "../Exceptions.h"
#include "../analysis/Analyser.h"

#define WINDOW_TITLE  "Speech analysis"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define WINDOW_FONT     "fonts/open-sans/OpenSans-Regular.ttf"
#define WINDOW_FONTSIZE 24

class AnalyserWindow {
public:
    explicit AnalyserWindow(Analyser & analyser) noexcept(false);
    ~AnalyserWindow() noexcept;

    void mainLoop();

private:
    void preRender();
    void render();
    void renderGraph();

    void handleKeyDown(const Uint8 * state, SDL_Scancode scanCode);
    void handleKeyUp(const Uint8 * state, SDL_Scancode scanCode);
    void handleMouse(int x, int y);

    // Graphics-related members
    SDL_Window * window;
    SDL_Renderer * renderer;
    TTF_Font * font;
    FPSmanager fpsManager;

    int targetWidth, targetHeight;
    std::vector<SDL_Texture *> formantStrTex;
    SDL_Texture * pitchStrTex;
    SDL_Texture * lpOrderStrTex;
    SDL_Texture * maxFreqStrTex;

    // Rendering parameters
    bool renderRaw;
    int selectedFrame;
    double selectedFrequency;

    Analyser & analyser;

    friend class Analyser;

};


#endif //SPEECH_ANALYSIS_ANALYSERWINDOW_H
