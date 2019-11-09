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
        frequencies(k) = minFrequency + (static_cast<double>(k) * (maxFrequency - minFrequency)) / static_cast<double>(binCount);
        //frequencies(k) = std::pow(10.0, logMin + (static_cast<double>(k) * (logMax - logMin)) / static_cast<double>(binCount));
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
    Sint16 vx[binCount];
    Sint16 vy[binCount];

    for (int k = 0; k < binCount; ++k) {
        double freq = frequencies(k);
        double gain = spectrum(k);

        vx[k] = static_cast<Sint16>(width * (freq - minFrequency) / (maxFrequency - minFrequency));
        vy[k] = static_cast<Sint16>(height * (1 - 0.5 * gain));
    }

    // Set renderer target to the provided texture.
    int ret = SDL_SetRenderTarget(renderer, texture);
    if (ret < 0) {
        throw SDLException("Unable to set render target to texture");
    }

    // Render the spectrogram.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int k = 0; k < binCount - 1; ++k) {
        aalineRGBA(renderer, vx[k], vy[k], vx[k + 1], vy[k + 1], stroke.r, stroke.g, stroke.b, stroke.a);
    }

    SDL_RenderPresent(renderer);

    // Set renderer target back to default.
    ret = SDL_SetRenderTarget(renderer, nullptr);
    if (ret < 0) {
        throw SDLException("Unable to reset render target to default");
    }
}
