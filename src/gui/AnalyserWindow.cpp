//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include <sstream>
#include "AnalyserWindow.h"
#include "../Exceptions.h"
#include "SDLUtils.h"
#include "../signal/LPC.h"
#include "../signal/LPC_Frame.h"
#include "../signal/Filter.h"
#include "../signal/Pitch.h"
#include "../signal/Window.h"
#include "../signal/Resample.h"

using namespace Eigen;

AnalyserWindow::AnalyserWindow() noexcept(false) {
    targetWidth = WINDOW_WIDTH;
    targetHeight = WINDOW_HEIGHT;
    headerTex = nullptr;

    audioData.setZero(500);

    int ret;

    // Initialise graphics stuff.

    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0) {
        throw SDLException("Unable to initialise SDL video");
    }

    ret = TTF_Init();
    if (ret < 0) {
        throw TTFException("Unable to initialise TTF");
    }

    window = SDL_CreateWindow(
            WINDOW_TITLE,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        throw SDLException("Unable to create window");
    }

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        throw SDLException("Unable to create renderer");
    }

    font = TTF_OpenFont(WINDOW_FONT, WINDOW_FONTSIZE);
    if (font == nullptr) {
        throw TTFException("Unable to open font");
    }

}

AnalyserWindow::~AnalyserWindow() noexcept {

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

}

void AnalyserWindow::mainLoop() {

    initTextures();

    bool running = true;
    SDL_Event evt;

    int currentTime;
    int previousUpdate = 0;
    int previousRender = 0;

    while (running) {

        // Process events.
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                running = false;
            }
            else if (evt.type == SDL_KEYDOWN) {
                const Uint8 * state = SDL_GetKeyboardState(nullptr);
                if (state[SDL_SCANCODE_ESCAPE]) {
                    running = false;
                }
                else {
                    handleKeyDown(state);
                }
            }
            else if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                targetWidth = evt.window.data1;
                targetHeight = evt.window.data2;
            }
            else {
                // Ignore other events.
            }
        }

        if (!running) {
            continue;
        }

        currentTime = SDL_GetTicks();

        // Check for update.
        if (currentTime - previousUpdate > UPDATE_DELAY) {
            update();
            previousUpdate = SDL_GetTicks();
        }

        // Check for render.
        if (currentTime - previousRender > RENDER_DELAY) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            render();
            SDL_RenderPresent(renderer);

            previousRender = SDL_GetTicks();
        }

    }

}

void AnalyserWindow::initTextures() {
    SDL_Color headerFg{255, 255, 255};

    headerTex = SDL::renderText(renderer, font, "Speech analysis", headerFg);
    spectrumTex = SDL_CreateTexture(renderer,
                                   SDL_PIXELFORMAT_RGBA32,
                                   SDL_TEXTUREACCESS_TARGET,
                                   800,
                                   600);
}

void AnalyserWindow::render() {
    // Some variables for drawing.
    SDL_FPoint pos;

    // Draw header text.
    pos = {0.1, 0.05};
    SDL::renderRelativeToCenter(renderer, headerTex, targetWidth, targetHeight, &pos);

    // Draw spectrogram
    spectrogram.render(renderer, spectrumTex);
    pos = {0.5, 0.5};
    SDL::renderRelativeToCenter(renderer, spectrumTex, targetWidth, targetHeight, &pos);

    // Draw pitch estimation(s)
    SDL_Texture * pitchTex = SDL::renderText(renderer, font, pitchString.c_str(), {255,255,255});

    pos = {0.7, 0.05};
    SDL::renderRelativeToCenter(renderer, pitchTex, targetWidth, targetHeight, &pos);

    SDL_DestroyTexture(pitchTex);
}

void AnalyserWindow::update() {

    // Read captured audio.
    audioCapture.readBlock(audioData);

    // Downsample to 16kHz;
    double fs = 16000;
    ArrayXd x = Resample::resample(audioData, audioCapture.getSampleRate(), fs, 5);

    // Estimate pitch with two methods.
    std::stringstream builder(std::ios_base::out);
    Pitch::Estimation est{};

    Pitch::estimate_AMDF(x, fs, est);
    if (est.isVoiced) {
        builder << "Voiced: " << std::round(est.pitch) << " Hz";
    }
    else {
        builder << "Voiceless";
    }
    pitchString = builder.str();

    // Estimate LPC coefficients.
    const double preemph = 50.0;
    if (preemph < fs / 2.0) {
        Filter::preEmphasis(x, fs, preemph);
    }

    Window::applyBlackmanHarris(x);

    LPC::Frame lpc = { .nCoefficients = 18 };

    if (LPC::frame_auto(x, lpc)) {

        ArrayXcd h;
        Filter::responseFIR(lpc.a, spectrogram.getFrequencyArray(), fs, h);

        // Normalize.
        ArrayXd gain = abs(h);
        gain /= gain.maxCoeff();

        spectrogram.setSpectrumArray(gain);
    }

}

void AnalyserWindow::handleKeyDown(const Uint8 * state) {

}
