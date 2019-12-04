//
// Created by clo on 25/11/2019.
//

#include "../Analyser.h"
#include "../../lib/Signal/Filter.h"
#include "../../lib/Signal/Window.h"

using namespace Eigen;

void Analyser::preEmphGauss()
{
    constexpr double preEmphasisFrequency = 50.0;

    if (preEmphasisFrequency < fs / 2.0) {
        Filter::preEmphasis(x, fs, preEmphasisFrequency);
    }

    // Apply Hamming window.
    static ArrayXd gaussian(0);
    if (gaussian.size() != x.size()) {
        gaussian = Window::createHamming(x.size());
    }
    x *= gaussian;
}