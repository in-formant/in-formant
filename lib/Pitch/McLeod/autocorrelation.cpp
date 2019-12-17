//
// Created by rika on 22/11/2019.
//

#include "MPM.h"
#include "../../FFT/FFT.h"

using Eigen::Map, Eigen::ArrayXcd;

ArrayXd MPM::autocorrelation(Ref<const ArrayXd> x, int w) {

    const int N = x.size();

    fft_plan(N);
    ifft_plan(N);

    Map<ArrayXcd>(fft_in(N), N) = x;
    fft(N);
    Map<ArrayXcd> out(fft_out(N), N);

    Map<ArrayXcd>(ifft_in(N), N) = (out * conj(out)) / static_cast<double>(2 * N);
    ifft(N);
    return Map<ArrayXcd>(ifft_out(N), N).real();

}