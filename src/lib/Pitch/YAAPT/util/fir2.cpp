//
// Created by rika on 10/11/2019.
//

#include <iostream>
#include <fftw3.h>
#include "../YAAPT.h"
#include "../../../FFT/FFT.h"
#include "../../../Signal/Window.h"

using namespace Eigen;

void YAAPT::fir2(int n, const ArrayXd & basef, const ArrayXd & basem,
                 int grid_n, int ramp_n, ArrayXd & b) {
    // Default to Hamming window.
    ArrayXd window = Window::createHamming(n + 1);

    // Default grid size is 512... unless n + 1 >= 1024
    if (grid_n == 0) {
        grid_n = (n + 1 < 1024) ? 512 : (n + 1);
    }

    // ML behaviour appears to always round the grid size up to a power of 2
    grid_n = std::pow(2, std::ceil(std::log2(grid_n)));

    // Error out if the grid size is not big enough for the window
    if (2 * grid_n < n + 1) {
        std::cerr << "fir2: grid size must be greater than half the filter order" << std::endl;
        return;
    }


    if (ramp_n == 0) {
        ramp_n = std::floor(grid_n / 25);
    }

    // Remember original frequency points prior to applying ramps
    ArrayXd f = basef;
    ArrayXd m = basem;

    // Apply ramps to discontinuities
    if (ramp_n > 0) {
        // Separate identical frequencies, but keep the midpoint
        const int lenf = f.size();
        for (int i = 0; i < lenf - 1; ++i) {
            if (f(i) == f(i + 1)) {
                f(i) -= (double) ramp_n / (double) grid_n / 2.0;
                f(i + 1) += (double) ramp_n / (double) grid_n / 2.0;

                int modLenf = f.size();
                f.conservativeResize(modLenf + 1);
                f(modLenf) = basef(i);
            }
        }

        // Make sure the grid points stay monotonic in [0,1]
        f = f.max(0).min(1);
        std::sort(f.begin(), f.end());

        // Preserve window shape even though f may have changed
        interp1(basef, basem, f, m);
    }

    // Interpolate between grid points
    ArrayXd grid;
    interp1(f, m, ArrayXd::LinSpaced(grid_n + 1, 0.0, 1.0), grid);

    // Transform frequency response into time response and
    // center the response about n/2, truncating the excess
    if (n % 2 == 0) {
        ArrayXcd cx_b(grid.size() + grid_n - 1);
        auto plan = fftw_plan_dft_1d(cx_b.size(),
                                     (fftw_complex *) cx_b.data(),
                                     (fftw_complex *) cx_b.data(),
                                     FFTW_BACKWARD, 0);

        cx_b << grid, grid(seq(grid_n - 1, 1, -1));
        fftw_execute(plan);
        cx_b /= cx_b.size();

        double mid = (n + 1) / 2.0;
        b.resize(n + 1);
        b << cx_b(seq(last - floor(mid) + 1, last)).real(),
             cx_b(seq(0, std::ceil(mid) - 1)).real();

        fftw_destroy_plan(plan);
    }
    else {
        throw std::runtime_error("Unimplemented yet");

        /*ArrayXcd cx_b(grid.size() + 3 * grid_n - 1);
        auto plan = fftw_plan_dft_1d(cx_b.size(),
                                     (fftw_complex *) cx_b.data(),
                                     (fftw_complex *) cx_b.data(),
                                     FFTW_BACKWARD, 0);

        cx_b << grid, ArrayXcd::Zero(2 * grid_n), grid(seq(grid_n - 1, 1, -1));
        fftw_execute(plan);
        cx_b /= cx_b.size();

        b.resize(2 * n);
        b << cx_b(seq(last - n + 1, last, 2)).real(),
             cx_b(seq(1, n + 2, 2)).real();
        b *= 2;

        fftw_destroy_plan(plan);*/
    }

    b *= window;
}