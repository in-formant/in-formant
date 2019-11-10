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

    audioData.setZero(500);
    formantFrames.frames.resize(500);

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
            SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        throw SDLException("Unable to create window");
    }

    SDL_SetWindowMinimumSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);

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
    formantTex = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            1280, 720);

    formantTexCopy = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            1280, 720);
}

void AnalyserWindow::render() {
    // Some variables for drawing.
    SDL_FPoint pos;
    SDL_Rect src;
    SDL_FRect dst;
    SDL_Texture * pitchStrTex;
    SDL_Texture * formantStrTex;

    //--- BACKGROUND

    // Draw formant estimation tracks.
    pos = {0.6, 0.5};
    SDL::renderRelativeToCenter(renderer, formantTex, targetWidth, targetHeight, &pos);

    //--- FOREGROUND

    // Draw pitch estimation.
    pitchStrTex = SDL::renderText(renderer, font, pitchString.c_str(), {255, 255, 255, 255});

    pos = {0.9, 0.01};
    SDL::renderRelativeToCenter(renderer, pitchStrTex, targetWidth, targetHeight, &pos);

    SDL_DestroyTexture(pitchStrTex);

    // Draw formant estimation strings.
    for (int i = 0; i < formantString.size(); ++i) {
        formantStrTex = SDL::renderText(renderer, font, formantString.at(i).c_str(), {255, 255, 255, 255});

        pos = {0.01, static_cast<float>(0.02 + i * 0.06)};
        SDL::renderRelativeToCenter(renderer, formantStrTex, targetWidth, targetHeight, &pos);

        SDL_DestroyTexture(formantStrTex);
    }

}

void AnalyserWindow::update() {

    // Read captured audio.
    audioCapture.readBlock(audioData);
    double fs_orig = audioCapture.getSampleRate();
    ArrayXd x_orig = audioData;

    // Estimate pitch with AMDF, then refine with AMDF.
    Pitch::Estimation est{};

    std::stringstream pitchSB(std::ios_base::out);
    Pitch::estimate_AMDF(x_orig, fs_orig, est, 70, 1000, 1.2, 0.1);
    if (est.isVoiced && (est.pitch > 70 && est.pitch < 1000)) {
        double pitch = est.pitch;
        Pitch::estimate_AMDF(x_orig, fs_orig, est, 0.4 * pitch, 2.2 * pitch, 2.0, 0.2);
        if (est.isVoiced && (est.pitch > 0.4 * pitch && est.pitch < 2.2 * pitch)) {
            pitchSB << "Voiced: " << std::round(est.pitch) << " Hz";
        } else {
            est.isVoiced = false;
            pitchSB << "Voiceless";
        }
    }
    else {
        est.isVoiced = false;
        pitchSB << "Voiceless";
    }
    pitchString = pitchSB.str();

    // Downsample.
    double fs = 11000;
    ArrayXd x = Resample::resample(x_orig, fs_orig, fs, 50);

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
        const auto &Fi = fFrame.formant.at(i);

        std::stringstream formantSB(std::ios_base::out);
        formantSB << "F" << (i + 1) << " = ";
        formantSB << std::round(Fi.frequency) << " Hz";

        formantString[i] = formantSB.str();
    }

    formantFrames.frames.pop_back();
    formantFrames.frames.push_front(fFrame);

    // UPDATE FORMANT TEXTURE
    SDL_FRect dst;

    float xstep = 1280.0f / static_cast<float>(formantFrames.frames.size());

    SDL_SetRenderTarget(renderer, formantTexCopy);
    SDL_RenderClear(renderer);
    dst = {-xstep, 0, 1280, 720};
    SDL_RenderCopyF(renderer, formantTex, nullptr, &dst);
    SDL_RenderPresent(renderer);

    SDL_SetRenderTarget(renderer, formantTex);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, formantTexCopy, nullptr, nullptr);

    // Color differently if voicing detected or not.
    if (est.isVoiced) {
        SDL_SetRenderDrawColor(renderer, 255, 167, 0, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
    }
    for (const auto & formant : fFrame.formant) {
        SDL_RenderDrawPointF(renderer, 1280 - xstep, 720.0 - (600.0 * formant.frequency) / 5500.0);
    }

    if (est.isVoiced) {
        SDL_SetRenderDrawColor(renderer, 0, 167, 255, 255);
        dst = {1280 - xstep, static_cast<float>(720.0 - (600.0 * est.pitch) / 5500.0), xstep, 1};
        SDL_RenderFillRectF(renderer, &dst);
    }

    SDL_RenderPresent(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
}

void AnalyserWindow::handleKeyDown(const Uint8 * state) {

}
