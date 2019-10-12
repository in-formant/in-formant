//
// Created by clo on 12/09/2019.
//

#include "Spectrogram.h"
#include "SDLUtils.h"
#include "../Exceptions.h"

using namespace Eigen;

Spectrogram::Spectrogram(int minFrequency, int maxFrequency, int binCount) :
        minFrequency(minFrequency),
        maxFrequency(maxFrequency),
        binCount(binCount) {

    calculateFrequencyArray();

}

void Spectrogram::calculateFrequencyArray() {

    // Log10 scale.
    double logMin = std::log10(minFrequency);
    double logMax = std::log10(maxFrequency);

    spectrum.setZero(binCount);
    frequencies.resize(binCount);

    for (int k = 0; k < binCount; ++k) {
        //frequencies(k) = minFrequency + (static_cast<double>(k) * (maxFrequency - minFrequency)) / static_cast<double>(binCount);
        frequencies(k) = std::pow(10.0, logMin + (static_cast<double>(k) * (logMax - logMin)) / static_cast<double>(binCount));
    }

}

void Spectrogram::setMinFrequency(int minFreq) {
    minFrequency = minFreq;
    calculateFrequencyArray();
}

void Spectrogram::setMaxFrequency(int maxFreq) {
    maxFrequency = maxFreq;
    calculateFrequencyArray();
}

void Spectrogram::setBinCount(int count) {
    binCount = count;
    calculateFrequencyArray();
}

const ArrayXd & Spectrogram::getFrequencyArray() {
    return frequencies;
}

void Spectrogram::setSpectrumArray(const Eigen::ArrayXd & data) {
    spectrum = data;
}

void Spectrogram::render(SDL_Renderer * renderer, SDL_Texture * texture, SDL_Color stroke) {

    int width, height;
    SDL::queryTextureSize(texture, &width, &height);

    // Create the array of points.
    std::vector<SDL_FPoint> points(binCount);

    for (int k = 0; k < binCount; ++k) {

        SDL_FPoint & p = points.at(k);

        double freq = frequencies(k);
        double gain = spectrum(k);

        p.x = static_cast<float>(width * (freq - minFrequency) / (maxFrequency - minFrequency));
        p.y = static_cast<float>(height * (1 - 0.05 * gain));

    }

    // Render the spectrograph.

    int ret;

    ret = SDL_SetRenderTarget(renderer, texture);
    if (ret < 0) {
        throw SDLException("Unable to set render target to texture");
    }

    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, stroke.r, stroke.g, stroke.b, stroke.a);
    SDL_RenderDrawLinesF(renderer, points.data(), points.size());

    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    SDL_RenderPresent(renderer);

    ret = SDL_SetRenderTarget(renderer, nullptr);
    if (ret < 0) {
        throw SDLException("Unable to reset render target to default");
    }

}
