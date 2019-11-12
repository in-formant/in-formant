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
#include "../lib/Pitch/YAAPT/YAAPT.h"

using namespace Eigen;

AnalyserWindow::AnalyserWindow() noexcept(false) {
    targetWidth = WINDOW_WIDTH;
    targetHeight = WINDOW_HEIGHT;

    audioData.setZero(500);
    rawFormantTrack.frames.resize(250, {5, {{550}, {1650}, {2750}, {3850}, {4950}}});
    pitchTrack.resize(250, 0);

    renderRaw = false;
    pauseScroll = false;

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
                    handleKey(state);
                }
            }
            else if (evt.type == SDL_KEYUP) {
                const Uint8 * state = SDL_GetKeyboardState(nullptr);
                handleKey(state);
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
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            render();
            SDL_RenderPresent(renderer);

            previousRender = SDL_GetTicks();
        }

    }

}

void AnalyserWindow::initTextures() {
    rawFormantTex = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            1280, 720);
    rawFormantTexCopy = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            1280, 720);

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
    SDL::renderRelativeToCenter(renderer, renderRaw ? rawFormantTex : formantTex, targetWidth, targetHeight, &pos);

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
        /*double pitch = est.pitch;
        Pitch::estimate_AMDF(x_orig, fs_orig, est, 0.4 * pitch, 2.2 * pitch, 2.0, 0.2);
        if (est.isVoiced && (est.pitch > 0.4 * pitch && est.pitch < 2.2 * pitch)) {*/
            pitchSB << "Voiced: " << std::round(est.pitch) << " Hz";
        /*} else {
            est.isVoiced = false;
            pitchSB << "Voiceless";
        }*/
    }
    else {
        est.isVoiced = false;
        pitchSB << "Voiceless";
    }

    /*YAAPT::Result yaaptEst;
    YAAPT::Params yaaptPar;
    YAAPT::getF0_slow(x_orig, fs_orig, yaaptEst, yaaptPar);
    pitchSB << "YAAPT: avg = " << round(yaaptEst.pitch);*/

    pitchTrack.pop_front();
    pitchTrack.push_back(est.isVoiced ? est.pitch : 0.0);

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
            LPC::Auto);
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

    rawFormantTrack.frames.pop_front();
    rawFormantTrack.frames.push_back(fFrame);

    // Track formants.
    int maxnFormants = 0;
    for (int i = 0; i < rawFormantTrack.frames.size(); ++i) {
        maxnFormants = std::max(maxnFormants, rawFormantTrack.frames[i].nFormants);
    }

    Formant::tracker(
            rawFormantTrack, formantTrack, maxnFormants, 4,
            550, 1650, 2750, 3850, 4950,
            1.0, 1.0, 1.0);

    SDL_Rect src, dst;
    int xstep = 4;

    // CHECK TO UPDATE TEXTURES
    if (!pauseScroll) {

        // UPDATE RAW FORMANT TEXTURE

        SDL_SetRenderTarget(renderer, rawFormantTexCopy);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        dst = {-xstep, 0, 1280, 720};
        SDL_RenderCopy(renderer, rawFormantTex, nullptr, &dst);
        SDL_RenderPresent(renderer);

        SDL_SetRenderTarget(renderer, rawFormantTex);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, rawFormantTexCopy, nullptr, nullptr);

        // Color differently if voicing detected or not.
        if (est.isVoiced) {
            SDL_SetRenderDrawColor(renderer, 255, 167, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
        }
        for (const auto &formant : fFrame.formant) {
            SDL_RenderDrawPoint(renderer, 1280 - xstep, 720 - (600 * formant.frequency) / 5500);
        }

        if (est.isVoiced) {
            SDL_SetRenderDrawColor(renderer, 0, 167, 255, 255);
            dst = {1280 - xstep, static_cast<int>(720 - (600 * est.pitch) / 5500), xstep, 1};
            SDL_RenderFillRect(renderer, &dst);
        }

        SDL_RenderPresent(renderer);
        SDL_SetRenderTarget(renderer, nullptr);

        // UPDATE FORMANT TEXTURE

        int nframes = formantTrack.frames.size();
        int xoff = xstep * nframes;

        SDL_SetRenderTarget(renderer, formantTexCopy);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        src = {0, 0, 1280 - xoff, 720};
        dst = {-xstep, 0, 1280 - xoff, 720};
        SDL_RenderCopy(renderer, formantTex, &src, &dst);
        SDL_RenderPresent(renderer);

        SDL_SetRenderTarget(renderer, formantTex);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, formantTexCopy, nullptr, nullptr);

        // Color differently if voicing detected or not.
        int xt = 1280 - xoff - 1;
        for (int iframe = 0; iframe < nframes - 1; ++iframe) {
            const auto &curFrame = formantTrack.frames.at(iframe);
            const auto &nextFrame = formantTrack.frames.at(iframe + 1);
            const double pitch = pitchTrack.at(iframe);

            int r, g, b;
            if (pitch > 0) {
                SDL_SetRenderDrawColor(renderer, 255, 167, 0, 255);
                r = 255;
                g = 167;
                b = 0;
            } else {
                SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
                r = 139;
                g = 0;
                b = 0;
            }

            int nfrm = 0;
            for (const auto &formant : curFrame.formant) {
                if (nfrm < nextFrame.nFormants) {
                    double freq = nextFrame.formant[nfrm].frequency;
                    if (std::max(freq, formant.frequency) / std::min(freq, formant.frequency) < 1.5) {
                        lineRGBA(renderer,
                                 xt, 720 - (600 * formant.frequency) / 5500,
                                 xt + xstep - 1, 720 - (600 * nextFrame.formant[nfrm].frequency) / 5500,
                                 r, g, b, 255);
                    } else {
                        SDL_RenderDrawPoint(renderer, xt, 720 - (600 * formant.frequency) / 5500);
                    }
                } else {
                    SDL_RenderDrawPoint(renderer, xt, 720 - (600 * formant.frequency) / 5500);
                }
                nfrm++;
            }

            if (pitch > 0) {
                SDL_SetRenderDrawColor(renderer, 0, 167, 255, 255);
                dst = {xt, static_cast<int>(720 - (600 * pitch) / 5500), xstep, 1};
                SDL_RenderFillRect(renderer, &dst);
            }

            xt += xstep;
        }

        SDL_RenderPresent(renderer);
        SDL_SetRenderTarget(renderer, nullptr);

    }
}

void AnalyserWindow::handleKey(const Uint8 * state) {
    renderRaw = state[SDL_SCANCODE_R];
    pauseScroll = state[SDL_SCANCODE_P];
}
