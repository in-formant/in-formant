//
// Created by clo on 7/12/2019.
//

#include "../Analyser.h"
#include "FFT/FFT.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"

using namespace Eigen;

void Analyser::analyseSpectrum()
{
    const int N = x.size();

    rfft_plan(nfft);

    Map<ArrayXd> xin(rfft_in(nfft), nfft);

    if (nfft < N) {
        // Grab the center segment of the signal (to respect windowing).
        xin = x.segment(N / 2 - nfft / 2, nfft) * Window::createBlackmanHarris(nfft);
    }
    else if (nfft > N) {
        // Zero-pad the signal.
        xin.head(N) = x * Window::createBlackmanHarris(N);
        xin.tail(nfft - N).setZero();
    }
    else {
        xin = x * Window::createBlackmanHarris(N);
    }

    rfft(nfft);

    Map<ArrayXd> xout(rfft_out(nfft), nfft);

    lastSpectrumFrame.fs = fs;
    lastSpectrumFrame.nfft = nfft;
    lastSpectrumFrame.spec = xout;
}
