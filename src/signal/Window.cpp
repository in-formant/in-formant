//
// Created by rika on 14/09/2019.
//

#include "Window.h"

using namespace Eigen;

void Window::applyHamming(ArrayXd & x) {
    int L = x.size();

    static constexpr double a0 = 0.53836;
    static constexpr double a1 = 0.46164;

    for (int k = 0; k < L; ++k) {
        x(k) *= (a0 - a1 * cos((2.0 * M_PI * k) / static_cast<double>(L - 1)));
    }
}

void Window::applyHanning(ArrayXd & x) {
    int L = x.size();

    for (int k = 0; k < L; ++k) {
        x(k) *= (0.5 - 0.5 * cos((2.0 * M_PI * k) / static_cast<double>(L - 1)));
    }
}

void Window::applyBlackmanHarris(ArrayXd & x) {
    int L = x.size();

    static constexpr double a0 = 0.35875;
    static constexpr double a1 = 0.48829;
    static constexpr double a2 = 0.14128;
    static constexpr double a3 = 0.01168;

    for (int k = 0; k < L; ++k) {
        x(k) *= (a0
                - a1 * cos((2.0 * M_PI * k) / static_cast<double>(L - 1))
                + a2 * cos((4.0 * M_PI * k) / static_cast<double>(L - 1))
                - a3 * cos((6.0 * M_PI * k) / static_cast<double>(L - 1)));
    }
}

ArrayXd Window::createGaussian(int size) {

    ArrayXd win(size);
    double imid = 0.5 * size, edge = std::exp(-12.0);
    for (int i = 1; i <= size; ++i) {
        win(i - 1) = (std::exp(-48.0 * (i - imid) * (i - imid) / (size + 1)) - edge) / (1 - edge);
    }
    return win;
}