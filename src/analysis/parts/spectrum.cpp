//
// Created by clo on 7/12/2019.
//

#include "../Analyser.h"
#include "../../lib/FFT/FFT.h"
#include "../../lib/Signal/Filter.h"
#include "../../lib/Signal/Window.h"

using namespace Eigen;

void Analyser::analyseSpectrum()
{
    const int N = x.size();

    rfft_plan(nfft);

    Map<ArrayXd> xin(rfft_in(nfft), nfft);

    if (nfft < N) {
        // Grab the center segment of the signal (to respect windowing).
        xin = x.segment(N / 2 - nfft / 2, nfft);
    }
    else if (nfft > N) {
        // Zero-pad the signal.
        xin.head(N) = x;
        xin.tail(nfft - N).setZero();
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
