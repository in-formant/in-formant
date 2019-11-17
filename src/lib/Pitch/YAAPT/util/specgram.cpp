//
// Created by rika on 10/11/2019.
//

#include "../YAAPT.h"
#include "../../../FFT/kissfft.hh"

void YAAPT::specgram(const std::array<Eigen::ArrayXd, numFrames> & x, int n,
              Eigen::ArrayXXcd & S)
{
    using namespace Eigen;

    int windowSize = x[0].size();

    ArrayXd window = Window::createHanning(windowSize);

    if (x[0].size() < windowSize) {
        throw std::runtime_error("len(x) < len(win)");
    }

    S.resize(n, numFrames);

    ArrayXcd in(n), out(n);
    kissfft<double> fft(n, false);

    for (int i = 0; i < numFrames; ++i) {
        in.setZero();
        in.head(windowSize) = x[i] * window;
        fft.transform(in.data(), out.data());

        S.col(i) = out;
    }

}