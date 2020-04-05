//
// Created by clo on 25/11/2019.
//

#include "../Analyser.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"

using namespace Eigen;

void Analyser::applyWindow()
{
    // Apply Hanning window.
    static ArrayXd win(0);
    if (win.size() != x.size()) {
        win = Window::createGaussian(x.size());
    }
    x *= win;
}

void Analyser::applyPreEmphasis()
{
    constexpr double preEmphasisFrequency = 500.0;

    if (preEmphasisFrequency < fs / 2.0) {
        Filter::preEmphasis(x, fs, preEmphasisFrequency);
        Filter::preEmphasis(x_fft, fs, preEmphasisFrequency);
    }
}
