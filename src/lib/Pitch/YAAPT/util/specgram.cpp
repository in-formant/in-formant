//
// Created by rika on 10/11/2019.
//

#include "../YAAPT.h"
#include "../../../FFT/FFT.h"

void YAAPT::specgram(const AudioFrames & x, int n, RefXXcd S)
{
    using namespace Eigen;

    int windowSize = x[0].size();

    ArrayXd window = Window::createHanning(windowSize);

    if (x[0].size() < windowSize) {
        throw std::runtime_error("len(x) < len(win)");
    }

    fft_plan(n);

    Map<ArrayXcd> in(fft_in(n), n);
    Map<ArrayXcd> out(fft_out(n), n);

    for (int i = 0; i < numFrames; ++i) {
        in.setZero();
        in.head(windowSize) = x[i] * window;
        fft(n);
        S.col(i) = out;
    }

}