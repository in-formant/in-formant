//
// Created by rika on 22/11/2019.
//

#include "YIN.h"
#include "../../FFT/FFT.h"

using namespace Eigen;

ArrayXd YIN::autocorrelation(Ref<const ArrayXd> x) {

    const int N = x.size();

    fft_plan(N);
    ifft_plan(N);

    Map<ArrayXcd>(fft_in(N), N) = x;
    fft(N);
    Map<ArrayXcd> out(fft_out(N), N);

    Map<ArrayXcd>(ifft_in(N), N) = (out * conj(out)) / static_cast<double>(N);
    ifft(N);
    return Map<ArrayXcd>(ifft_out(N), N).real() / static_cast<double>(N);

}
