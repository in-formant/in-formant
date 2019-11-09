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
    formantFrames.resize(1000, {550, 1650, 2750, 3850, 4950});

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

    // Draw header text.
    pos = {0.1, 0.05};
    SDL::renderRelativeToCenter(renderer, headerTex, targetWidth, targetHeight, &pos);

    // Draw pitch estimation.
    pitchTex = SDL::renderText(renderer, font, pitchString.c_str(), {255, 255, 255, 255});

    pos = {0.7, 0.05};
    SDL::renderRelativeToCenter(renderer, pitchTex, targetWidth, targetHeight, &pos);

    SDL_DestroyTexture(pitchTex);

    // Draw formant estimations.
    for (int i = 0; i < formantString.size(); ++i) {
        formantTex = SDL::renderText(renderer, font, formantString.at(i).c_str(), {255, 255, 255, 255});

        pos = {0.1, static_cast<float>(0.15 + i * 0.05)};
        SDL::renderRelativeToCenter(renderer, formantTex, targetWidth, targetHeight, &pos);

        SDL_DestroyTexture(formantTex);
    }

    const double n = formantFrames.size();
    const double xmin = targetWidth * 0.03;
    const double xmax = targetWidth * 0.97;
    const double ymin = targetHeight * 0.97;
    const double ymax = targetHeight * 0.2;

    SDL_SetRenderDrawColor(renderer, 255, 167, 0 ,255);

    SDL_Rect rect;
    rect.w = (xmax - xmin) / n + 1;
    rect.h = 1;

    double k = 0;
    for (const auto & fFrame : formantFrames) {
        rect.x = xmin + (k * (xmax - xmin)) / n;

        for (const double & frequency : fFrame) {
            rect.y = ymin + (frequency * (ymax - ymin)) / 6000.0;

            SDL_RenderFillRect(renderer, &rect);
        }

        k++;
    }
}

void AnalyserWindow::update() {

    // Read captured audio.
    audioCapture.readBlock(audioData);

    // Downsample.
    double fs = 12000;
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
    LPC::Frames lpc = LPC::analyse(
            x,
            11,
            30.0 / 1000.0,
            fs,
            50.0,
            LPC::Burg);
    LPC::Frame lpcFrame = lpc.d_frames.at(0);

    // Estimate formants.
    Formant::Frame fFrame;
    LPC::toFormantFrame(lpcFrame, fFrame, fs, 50.0);

    formantString.resize(fFrame.nFormants);
    for (int i = 0; i < fFrame.nFormants; ++i) {
        const auto & Fi = fFrame.formant.at(i);

        std::stringstream formantSB(std::ios_base::out);
        formantSB << "F" << (i + 1) << " = ";
        formantSB << std::round(Fi.frequency) << " Hz";

        formantString[i] = formantSB.str();
    }

    formantFrames.pop_front();
    std::vector<double> fFrameFreq(fFrame.nFormants);
    std::transform(fFrame.formant.begin(), fFrame.formant.end(), fFrameFreq.begin(), [](const auto & Fi) { return Fi.frequency; });
    formantFrames.push_back(fFrameFreq);

}

void AnalyserWindow::handleKeyDown(const Uint8 * state) {

}
