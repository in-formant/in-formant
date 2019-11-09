//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_SPECTROGRAM_H
#define SPEECH_ANALYSIS_SPECTROGRAM_H

#include <SDL2/SDL.h>
#include <Eigen/Core>

class Spectrogram {
public:
    explicit Spectrogram(int minFrequency = 20,
                         int maxFrequency = 6000,
                         int binCount = 4096);

    void setMinFrequency(int minFrequency);
    void setMaxFrequency(int maxFrequency);
    void setBinCount(int binCount);

    const Eigen::ArrayXd & getFrequencyArray();
    void setSpectrumArray(const Eigen::ArrayXd & data);

    void render(SDL_Renderer * renderer,
                SDL_Texture * texture,
                SDL_Color stroke = {255, 165, 0, 255});

private:
    void calculateFrequencyArray();

    int minFrequency;
    int maxFrequency;
    int binCount;

    Eigen::ArrayXd frequencies;
    Eigen::ArrayXd spectrum;
};


#endif //SPEECH_ANALYSIS_SPECTROGRAM_H
