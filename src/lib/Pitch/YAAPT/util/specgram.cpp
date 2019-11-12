//
// Created by rika on 10/11/2019.
//

#include <fftw3.h>
#include <iterator>
#include "../YAAPT.h"
#include "../../../Signal/Window.h"

using namespace Eigen;

void YAAPT::specgram(const ArrayXd & x, int n, int windowSize, int overlap,
                     ArrayXXcd & S)
{
    ArrayXd window = Window::createHanning(windowSize);
    int step = windowSize - overlap;

    if (x.size() <= windowSize) {
        throw std::runtime_error("len(x) <= len(win)");
    }

    // Build matrix of windowed data slices
    std::vector<Index> offset;

    int off = 0;
    while (off < x.size() - 1 - windowSize) {
        offset.push_back(off);
        off += step;
    }

    S.resize(n, offset.size());

    ArrayXcd data(n);
    auto plan = fftw_plan_dft_1d(n, (fftw_complex *) data.data(), (fftw_complex *) data.data(), FFTW_FORWARD, 0);

    for (int i = 0; i < offset.size(); ++i) {
        data.setZero();
        data.head(windowSize) = x.segment(offset[i], windowSize) * window;
        fftw_execute(plan);

        S.col(i) = data;
    }

    fftw_destroy_plan(plan);

}