//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include <sstream>
#include "AnalyserWindow.h"
#include "../Exceptions.h"
#include "SDLUtils.h"
#include "../lib/LPC/LPC.h"
#include "../lib/LPC/Frame/LPC_Frame.h"
#include "../lib/LPC/LPC_huber.h"
#include "../lib/Pitch/Pitch.h"
#include "../lib/Formant/Formant.h"
#include "../lib/Signal/Filter.h"
#include "../lib/Signal/Window.h"
#include "../lib/Signal/Resample.h"

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
    SDL_Texture * pitchTex;
    SDL_Texture * formantTex;

    // Draw spectrogram first.
    spectrogram.render(renderer, spectrumTex);
    pos = {0.5, 0.5};
    SDL::renderRelativeToCenter(renderer, spectrumTex, targetWidth, targetHeight, &pos);

    // Draw header text.
    pos = {0.1, 0.05};
    SDL::renderRelativeToCenter(renderer, headerTex, targetWidth, targetHeight, &pos);

    // Draw pitch estimation.
    pitchTex = SDL::renderText(renderer, font, pitchString.c_str(), {255,255,255});

    pos = {0.7, 0.05};
    SDL::renderRelativeToCenter(renderer, pitchTex, targetWidth, targetHeight, &pos);

    SDL_DestroyTexture(pitchTex);

    // Draw formant estimations.
    for (int i = 0; i < formantString.size(); ++i) {
        formantTex = SDL::renderText(renderer, font, formantString.at(i).c_str(), {255, 255, 255});

        pos = {0.1, 0.2f + i * 0.05f};
        SDL::renderRelativeToCenter(renderer, formantTex, targetWidth, targetHeight, &pos);

        SDL_DestroyTexture(formantTex);
    }
}

void AnalyserWindow::update() {

    // Read captured audio.
    audioCapture.readBlock(audioData);

    // Downsample.
    double fs = 10000;
    ArrayXd x = Resample::resample(audioData, audioCapture.getSampleRate(), fs, 50);

    // Estimate pitch with two methods.
    Pitch::Estimation est{};

    std::stringstream pitchSB(std::ios_base::out);
    Pitch::estimate_AMDF(x, fs, est);
    if (est.isVoiced) {
        pitchSB << "Voiced: " << std::round(est.pitch) << " Hz";
    }
    else {
        pitchSB << "Voiceless";
    }
    pitchString = pitchSB.str();

    // Estimate LPC coefficients.
    /*LPC::Frames lpc = LPC::analyse(
            x,
            5,
            30.0 / 1000.0,
            fs,
            20.0,
            LPC::Burg);
    LPC::Frame lpcFrame = lpc.d_frames.at(0);*/

    LPC::Frame lpcFrame = {.nCoefficients = 11};
    LPC::frame_burg(x, lpcFrame);

    // Calculate filter response for spectrogram.
    ArrayXcd h;
    Filter::responseFIR(lpcFrame.a,
                        spectrogram.getFrequencyArray(),
                        fs,
                        h);

    // Estimate formants.
    Formant::Frame fFrame;
    LPC::toFormantFrame(lpcFrame, fFrame, fs, 90.0);

    formantString.resize(fFrame.nFormants);
    for (int i = 0; i < fFrame.nFormants; ++i) {
        const auto & Fi = fFrame.formant.at(i);

        std::stringstream formantSB(std::ios_base::out);
        formantSB << "F" << (i + 1) << " = ";
        formantSB << Fi.frequency << " Hz  ";
        formantSB << "(band: " << Fi.bandwidth << " Hz)";

        formantString[i] = formantSB.str();
    }

    // Normalize.

    spectrogram.setSpectrumArray(abs(h));

}

void AnalyserWindow::handleKeyDown(const Uint8 * state) {

}
