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

    const int nfft = this->nfft / 2;

    rfft_plan(nfft);

    Map<ArrayXd> xin(rfft_in(nfft), nfft);

    if (nfft < N) {
        // Grab the center segment of the signal (to respect windowing).
        xin = x.segment(N / 2 - nfft / 2, nfft);
    }
    else if (nfft > N) {
        // Zero-pad the signal.
        xin.setZero();
        xin.segment(nfft / 2 - N / 2, N) = x;
    }
    else {
        xin = x;
    }

    xin *= Window::createBlackmanHarris(nfft);

    rfft(nfft);

    Map<ArrayXd> xout(rfft_out(nfft), nfft);

    lastSpectrumFrame.fs = fs;
    lastSpectrumFrame.nfft = nfft;
    lastSpectrumFrame.spec = xout;
}
