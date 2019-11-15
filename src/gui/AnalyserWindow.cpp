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

static constexpr double maxFrequency = 6000;
static constexpr Uint8 formantColors[9][3] = {
    {255, 167, 0},
    {255, 87, 217},
    {127, 255, 0},
    {87, 200, 200},
    {200, 167, 255},
    {0, 165, 156},
    {255, 255, 255}, // unused
    {255, 255, 255},
    {255, 255, 255},
};

AnalyserWindow::AnalyserWindow() noexcept(false) {
    targetWidth = WINDOW_WIDTH;
    targetHeight = WINDOW_HEIGHT;

    audioData.setZero(500);

    rawFormantTrack.frames.resize(500, {5, {{550}, {1650}, {2750}, {3850}, {4950}}});
    formantTrack.frames.resize(500, {5, {{550}, {1650}, {2750}, {3850}, {4950}}});
    pitchTrack.resize(500, 0);
    selectedFrame = 199;

    tailFormantLength = 20;
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
                    handleKeyDown(state, evt.key.keysym.scancode);
                }
            }
            else if (evt.type == SDL_KEYUP) {
                const Uint8 * state = SDL_GetKeyboardState(nullptr);
                handleKeyUp(state, evt.key.keysym.scancode);
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
            int x, y;
            SDL_GetMouseState(&x, &y);
            handleMouse(x, y);

            preRender();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            render();
            SDL_RenderPresent(renderer);

            previousRender = SDL_GetTicks();
        }

    }

}

void AnalyserWindow::initTextures() {
}

void AnalyserWindow::preRender() {
    const auto & frames = renderRaw ? rawFormantTrack : formantTrack;
    const auto & frame = frames.frames[selectedFrame];
    double pitch = pitchTrack[selectedFrame];

    std::stringstream sb(std::ios_base::out);
    const char * str;

    for (int i = 0; i < frame.nFormants; ++i) {
        const auto & Fi = frame.formant.at(i);

        sb << "F" << (i + 1) << " = ";
        sb << round(Fi.frequency) << " Hz";

        str = sb.str().c_str();

        formantStrTex.push_back(SDL::renderText(renderer, font, str,
                {formantColors[i][0], formantColors[i][1], formantColors[i][2], 255}));

        sb.str("");
        sb.clear();
    }

    if (pitch > 0) {
        sb << "Voiced: " << round(pitch) << " Hz";
    }
    else {
        sb << "Unvoiced";
    }

    str = sb.str().c_str();

    pitchStrTex = SDL::renderText(renderer, font, str, {255, 255, 255, 255});

    sb.str("");
    sb.clear();

}

void AnalyserWindow::render() {
    // Some variables for drawing.
    SDL_FPoint pos;

    //--- BACKGROUND
    renderGraph();

    //--- FOREGROUND

    // Draw formant estimation strings.
    for (int i = 0; i < formantStrTex.size(); ++i) {
        pos = {0.01, static_cast<float>(0.02 + i * 0.06)};
        SDL::renderRelativeToCenter(renderer, formantStrTex[i], targetWidth, targetHeight, &pos);
    }

    pos = {0.9, 0.01};
    SDL::renderRelativeToCenter(renderer, pitchStrTex, targetWidth, targetHeight, &pos);

    // Free textures.
    for (int i = 0; i < formantStrTex.size(); ++i) {
        SDL_DestroyTexture(formantStrTex[i]);
    }
    formantStrTex.clear();
    SDL_DestroyTexture(pitchStrTex);
}

void AnalyserWindow::update() {

    // Read captured audio.
    audioCapture.readBlock(audioData);
    double fs_orig = audioCapture.getSampleRate();
    ArrayXd x_orig = audioData;

    // Estimate pitch with AMDF, then refine with AMDF.
    Pitch::Estimation est{};

    Pitch::estimate_AMDF(x_orig, fs_orig, est, 70, 1000, 1.2, 0.01);
    if (est.isVoiced && (est.pitch > 70 && est.pitch < 1000)) {
        double pitch = est.pitch;
        Pitch::estimate_AMDF(x_orig, fs_orig, est, 0.8 * pitch, 1.1 * pitch, 2.0, 0.1);
        est.isVoiced &= (est.pitch > 0.4 * pitch && est.pitch < 2.2 * pitch);
    }
    else {
        est.isVoiced = false;
    }

    /*YAAPT::Result yaaptEst;
    YAAPT::Params yaaptPar;
    YAAPT::getF0_slow(x_orig, fs_orig, yaaptEst, yaaptPar);
    pitchSB << "YAAPT: avg = " << round(yaaptEst.pitch);*/

    // Only read a 25ms frame. Downsample.
    double fs = maxFrequency * 2;
    ArrayXd x = Resample::resample(x_orig, fs_orig, fs, 50);

    // Estimate LPC coefficients.
    LPC::Frames lpc = LPC::analyse(
            x,
            10,
            30.0 / 1000.0,
            fs,
            50.0,
            LPC::Auto);
    LPC::Frame lpcFrame = lpc.d_frames.at(0);

    // Estimate formants.
    Formant::Frame fFrame{};
    LPC::toFormantFrame(lpcFrame, fFrame, fs, 50.0);

    if (!pauseScroll) {
        pitchTrack.pop_front();
        rawFormantTrack.frames.pop_front();
        formantTrack.frames.pop_front();

        pitchTrack.push_back(est.isVoiced ? est.pitch : 0);
        rawFormantTrack.frames.push_back(fFrame);
        formantTrack.frames.push_back(fFrame);
    }

    // Track formants. We'll only look at a small amount of trailing formant frames to save CPU load.
    // The number will be truncated to the largest number of consecutive voiced frames.
    /*Formant::Frames tailRawFrames, tailFrames;
    std::copy(rawFormantTrack.frames.end() - tailFormantLength, rawFormantTrack.frames.end(), std::back_inserter(tailRawFrames.frames));

    int maxnFormants = 0;
    for (const auto &frame : tailRawFrames.frames) {
        maxnFormants = std::max(maxnFormants, frame.nFormants);
    }

    Formant::tracker(
            tailRawFrames, tailFrames, maxnFormants, 2,
            550, 1650, 2750, 3850, 4950,
            1.0, 1.0, 1.0);

    std::copy(tailFrames.frames.begin(), tailFrames.frames.end(), formantTrack.frames.end() - tailFormantLength);*/
}

void AnalyserWindow::handleKeyDown(const Uint8 * state, SDL_Scancode scanCode) {
    renderRaw = state[SDL_SCANCODE_R];
}

void AnalyserWindow::handleKeyUp(const Uint8 * state, SDL_Scancode scanCode) {
    renderRaw = state[SDL_SCANCODE_R];
    if (scanCode == SDL_SCANCODE_P) {
        pauseScroll = !pauseScroll;
    }
}

void AnalyserWindow::handleMouse(int x, int y) {
    const int nframe = formantTrack.frames.size();
    const double xstep = static_cast<double>(targetWidth) / static_cast<double>(nframe - 1);

    selectedFrame = std::min<int>(std::max<int>(std::round(x / xstep), 0), nframe - 1);
}

void AnalyserWindow::renderGraph() {

    const int nframe = formantTrack.frames.size();
    const double xstep = static_cast<double>(targetWidth) / static_cast<double>(nframe);

    double x, y;

    x = 0;
    for (int iframe = 0; iframe < nframe; ++iframe) {
        double pitch = pitchTrack[iframe];
        const Formant::Frame * frame;
        if (renderRaw) {
            frame = &rawFormantTrack.frames[iframe];
        } else {
            frame = &formantTrack.frames[iframe];
        }

        int formantNb = 0;
        for (const auto & formant : frame->formant) {
            y = targetHeight - (targetHeight * formant.frequency) / maxFrequency;

            if (pitch > 0) {
                filledCircleRGBA(renderer, x, y, 1.25 * xstep, formantColors[formantNb][0], formantColors[formantNb][1], formantColors[formantNb][2], 255);
                //filledCircleRGBA(renderer, x, y, xstep / 2, 255,167, 0, 255);
            }
            else {
                for (int xp = x; xp < x + xstep - 1; ++xp) {
                    pixelRGBA(renderer, xp, y, 139, 0, 0, 196);
                }
            }

            formantNb++;
        }

        if (pitch > 0) {
            y = targetHeight - (targetHeight * pitch) / maxFrequency;
            boxRGBA(renderer, x, y - 1, x + xstep - 1, y + 1, 0, 167, 255, 255);
        }

        x += xstep;
    }

    // Draw a vertical line where the selected frame is.
    vlineRGBA(renderer, selectedFrame * xstep, 0, targetHeight, 127,  127, 127, 127);
}
