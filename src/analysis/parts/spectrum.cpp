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
    rfft_plan(nfft);

    Map<ArrayXd>(rfft_in(nfft), nfft) = x_fft.head(nfft) * Window::createHanning(nfft); 

    rfft(nfft);

    Map<ArrayXd> xout(rfft_out(nfft), nfft);

    lastSpectrumFrame.fs = audioCapture->getSampleRate();
    lastSpectrumFrame.nfft = nfft;
    lastSpectrumFrame.spec = xout;
}
