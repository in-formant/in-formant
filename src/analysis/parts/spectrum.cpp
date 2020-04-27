//
// Created by clo on 7/12/2019.
//

#include <iostream>
#include "../Analyser.h"
#include "FFT/FFT.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"
#include "Signal/Resample.h"

using namespace Eigen;

void Analyser::analyseSpectrum()
{
    // Speech signal spectrum
    
    static ArrayXd win;
    if (win.size() != nfft) {
        win = Window::createHanning(nfft);
    }

    rfft_plan(nfft);

    Map<ArrayXd> xin(rfft_in(nfft), nfft);
    Map<ArrayXd> xout(rfft_out(nfft), nfft);
    
    xin = x_fft.head(nfft) * win;

    rfft(nfft);
    
    lastSpectrumFrame.fs = audioInterface->getRecordSampleRate();
    lastSpectrumFrame.nfft = nfft;
    lastSpectrumFrame.spec = xout;

    // LPC spectrum

    constexpr int nfftLpc = 128;
    const int p = lpcFrame.nCoefficients;

    rfft_plan(nfftLpc);

    Map<ArrayXd> yin(rfft_in(nfftLpc), nfftLpc);
    Map<ArrayXd> yout(rfft_out(nfftLpc), nfftLpc);

    ArrayXd h;

    yin.setZero();
    yin(0) = 1.0;
    yin.segment(1, p) = lpcFrame.a;

    rfft(nfftLpc);

    h = 0.5 / yout;

    lpcSpectrum.fs = fs;
    lpcSpectrum.nfft = nfftLpc;
    lpcSpectrum.spec = h;
    
}
