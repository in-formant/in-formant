//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AnalyserWindow.h"
#include "../Exceptions.h"
#include "SDLUtils.h"

using namespace Eigen;

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

AnalyserWindow::AnalyserWindow(Analyser & analyser) noexcept(false)
    : targetWidth(WINDOW_WIDTH),
      targetHeight(WINDOW_HEIGHT),
      selectedFrame(analysisFrameCount - 1),
      renderRaw(false),
      renderLogScale(true),
      analyser(analyser)
{
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
            WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (window == nullptr) {
        throw SDLException("Unable to create window");
    }

    SDL_SetWindowMinimumSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (renderer == nullptr) {
        throw SDLException("Unable to create renderer");
    }

    font = TTF_OpenFont(WINDOW_FONT, WINDOW_FONTSIZE);
    if (font == nullptr) {
        throw TTFException("Unable to open font");
    }

    SDL_initFramerate(&fpsManager);
    SDL_setFramerate(&fpsManager, 60);
}

AnalyserWindow::~AnalyserWindow() noexcept {

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

}

void AnalyserWindow::mainLoop() {

    bool running = true;
    SDL_Event evt;

    while (running) {

        // Process events.
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                running = false;
            } else if (evt.type == SDL_KEYDOWN) {
                const Uint8 *state = SDL_GetKeyboardState(nullptr);
                if (state[SDL_SCANCODE_ESCAPE]) {
                    running = false;
                } else {
                    handleKeyDown(state, evt.key.keysym.scancode);
                }
            } else if (evt.type == SDL_KEYUP) {
                const Uint8 *state = SDL_GetKeyboardState(nullptr);
                handleKeyUp(state, evt.key.keysym.scancode);
            } else if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                targetWidth = evt.window.data1;
                targetHeight = evt.window.data2;
            } else {
                // Ignore other events.
            }
        }

        if (!running) {
            continue;
        }

        int x, y;
        SDL_GetMouseState(&x, &y);
        handleMouse(x, y);

        preRender();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render();
        SDL_RenderPresent(renderer);

        SDL_framerateDelay(&fpsManager);

    }

    analyser.stopThread();

}

void AnalyserWindow::preRender() {
    const auto & frame = analyser.getFormantFrame(selectedFrame, renderRaw);
    double pitch = analyser.getPitchFrame(selectedFrame);

    char str[64];

    for (int i = 0; i < frame.nFormants; ++i) {
        const auto & Fi = frame.formant.at(i);

        sprintf(str, "F%d = %.0f Hz", (i + 1), round(Fi.frequency));

        formantStrTex.push_back(SDL::renderText(renderer, font, str,
                {formantColors[i][0], formantColors[i][1], formantColors[i][2], 255}));
    }

    if (pitch > 0)
        sprintf(str, "Voiced: %.0f Hz", round(pitch));
    else
        sprintf(str, "Unvoiced");
    pitchStrTex = SDL::renderText(renderer, font, str, {255, 255, 255, 255});

    sprintf(str, "LP order: %d", analyser.getLinearPredictionOrder());
    lpOrderStrTex = SDL::renderText(renderer, font, str, {187, 187, 187, 255});

    sprintf(str, "F.max: %.0f Hz", round(analyser.getMaximumFrequency()));
    maxFreqStrTex = SDL::renderText(renderer, font, str, {187, 187, 187, 255});

}

void AnalyserWindow::render() {
    // Some variables for drawing.
    SDL_FPoint pos;

    //--- BACKGROUND
    renderGraph();

    //--- FOREGROUND

    // Draw formant estimation strings.
    for (int i = 0; i < formantStrTex.size(); ++i) {
        pos = {0.01, static_cast<float>(0.01 + i * 0.05)};
        SDL::renderRelative(renderer, formantStrTex[i], targetWidth, targetHeight, &pos);
    }

    pos = {0.8, 0.01};
    SDL::renderRelative(renderer, pitchStrTex, targetWidth, targetHeight, &pos);

    pos = {0.15, 0.01};
    SDL::renderRelative(renderer, lpOrderStrTex, targetWidth, targetHeight, &pos);

    pos = {0.15, 0.06};
    SDL::renderRelative(renderer, maxFreqStrTex, targetWidth, targetHeight, &pos);

    // Free textures.
    for (int i = 0; i < formantStrTex.size(); ++i) {
        SDL_DestroyTexture(formantStrTex[i]);
    }
    formantStrTex.clear();
    SDL_DestroyTexture(pitchStrTex);
    SDL_DestroyTexture(lpOrderStrTex);
    SDL_DestroyTexture(maxFreqStrTex);
}

void AnalyserWindow::handleKeyDown(const Uint8 * state, SDL_Scancode scanCode) {
    renderRaw = state[SDL_SCANCODE_R];
}

void AnalyserWindow::handleKeyUp(const Uint8 * state, SDL_Scancode scanCode) {
    renderRaw = state[SDL_SCANCODE_R];
    if (scanCode == SDL_SCANCODE_P) {
        analyser.toggle();
    }
    else if (scanCode == SDL_SCANCODE_S) {
        renderLogScale = !renderLogScale;
    }

    int parIncr = (scanCode == SDL_SCANCODE_UP ? 1
                    : (scanCode == SDL_SCANCODE_DOWN ? -1 : 0));
    if (parIncr != 0) {
        // LPC Order
        if (state[SDL_SCANCODE_L]) {
            int val = analyser.getLinearPredictionOrder();
            analyser.setLinearPredictionOrder(val + parIncr);
        }
        else if (state[SDL_SCANCODE_F]) {
            double val = analyser.getMaximumFrequency();
            analyser.setMaximumFrequency(val + 100.0 * parIncr);
        }
    }
}

void AnalyserWindow::handleMouse(int x, int y) {
    constexpr int nframe = analysisFrameCount;
    const double maximumFrequency = analyser.getMaximumFrequency();

    selectedFrame = std::clamp<int>((x * nframe) / targetWidth, 0, nframe - 1);
    selectedFrequency = std::clamp<double>(frequencyFromY(y), 0.0, maximumFrequency);
}

int AnalyserWindow::yFromFrequency(double frequency) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (renderLogScale) {
        const double maxMel = 2595 * std::log10(1 + maximumFrequency / 700.0);
        const double mel = 2595 * std::log10(1 + frequency / 700.0);

        return (targetHeight * (maxMel - mel)) / maxMel;
    }
    else {
        return (targetHeight * (maximumFrequency - frequency)) / maximumFrequency;
    }
}

double AnalyserWindow::frequencyFromY(int y) {
    const double maximumFrequency = analyser.getMaximumFrequency();

    if (renderLogScale) {
        const double maxMel = 2595 * std::log10(1 + maximumFrequency / 700.0);
        const double mel = maxMel - (y * maxMel) / targetHeight;

        return 700.0 * (std::pow(10.0, mel / 2595.0) - 1);
    }
    else {
        return maximumFrequency - (y * maximumFrequency) / targetHeight;
    }
}

void AnalyserWindow::renderGraph() {
    constexpr int nframe = analysisFrameCount;
    const int xstep = std::max(nframe / targetWidth, 1);

    const double maximumFrequency = analyser.getMaximumFrequency();

    const int formantRadius = 2;
    const int ruleSmall = 4;
    const int ruleBig = 8;

    int x, y;

    for (int iframe = 0; iframe < nframe; ++iframe) {
        const auto &frame = analyser.getFormantFrame(iframe, renderRaw);
        double pitch = analyser.getPitchFrame(iframe);

        x = (iframe * targetWidth) / nframe;

        int formantNb = 0;
        for (const auto &formant : frame.formant) {
            y = yFromFrequency(formant.frequency);

            if (pitch > 0) {
                filledCircleRGBA(renderer, x, y, formantRadius, formantColors[formantNb][0],
                                 formantColors[formantNb][1], formantColors[formantNb][2], 255);
                //filledCircleRGBA(renderer, x, y, xstep / 2, 255,167, 0, 255);
            } else {
                boxRGBA(renderer, x, y - 1, x + xstep - 1, y + 1, 139, 0, 0, 255);
            }

            formantNb++;
        }

        if (pitch > 0) {
            y = yFromFrequency(pitch);
            boxRGBA(renderer, x, y - 2, x, y + 1, 0, 167, 255, 255);
        }

        x += xstep;
    }

    char str[32];

    for (double frequency = 0.0; frequency <= maximumFrequency; frequency += 100.0) {
        y = yFromFrequency(frequency);

        if (fmod(frequency, 500.0) >= 1e-10) {
            boxRGBA(renderer, targetWidth - ruleSmall, y - 1, targetWidth, y + 1, 187, 187, 187, 127);
        }
        else {
            sprintf(str, "%.0f", frequency);

            boxRGBA(renderer, targetWidth - ruleBig, y - 2, targetWidth, y + 1, 187, 187, 187, 255);
            if (frequency == 0.0) {
                y -= 9;
            } else if (frequency == maximumFrequency) {
                y += 2;
            } else {
                y -= 3;
            }

            gfxPrimitivesSetFont(nullptr, 0, 0);
            stringRGBA(renderer, targetWidth - ruleBig - 9 * (frequency > 0.0 ? floor(log10(frequency) + 1) : 1), y, str, 187, 187, 187, 255);
        }
    }

    x = (selectedFrame * targetWidth) / nframe;
    y = yFromFrequency(selectedFrequency);

    // Draw a vertical line where the selected frame is.
    vlineRGBA(renderer, x, 0, targetHeight, 127, 127, 127, 127);
    hlineRGBA(renderer, 0, targetWidth, y, 127, 127, 127, 127);
    // Draw freq string right next to it.
    sprintf(str, "%.0f Hz", selectedFrequency);
    gfxPrimitivesSetFont(nullptr, 0, 0);
    stringRGBA(renderer, targetWidth - 9 * (8 + (selectedFrequency > 0.0 ? floor(log10(selectedFrequency) + 1) : 1)), y - 13, str, 255, 255, 255, 255);
}
