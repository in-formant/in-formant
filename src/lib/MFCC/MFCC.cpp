//
// Created by clo on 25/11/2019.
//

#include "MFCC.h"
#include "../FFT/FFT.h"
#include "../Signal/Filter.h"

using namespace Eigen;

ArrayXXd MFCC::initDCT(int numCepstra, int numFilters)
{
    ArrayXd v1(numCepstra + 1);
    ArrayXd v2(numFilters);
    for (int i = 0; i <= numCepstra; ++i)
        v1(i) = i;
    for (int i = 0; i < numFilters; ++i)
        v2(i) = i + 0.5;

    ArrayXXd dct(numCepstra + 1, numFilters);
    double c = sqrt(2.0 / static_cast<double>(numFilters));
    for (int i = 0; i <= numCepstra; ++i) {
        for (int j = 0; j < numFilters; ++j) {
            dct(i, j) = c * cos(M_PI / static_cast<double>(numFilters) * v1[i] * v2[j]);
        }
    }

    return dct;
}

ArrayXXd MFCC::initFilterBank(double lowFreq, double highFreq, double fs, int numFilters, int numFftBins)
{
    double lowFreqMel = hz2mel(lowFreq);
    double highFreqMel = hz2mel(highFreq);

    ArrayXd filterCentreFreq(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        filterCentreFreq(i) = mel2hz(lowFreqMel + (highFreqMel - lowFreqMel) / static_cast<double>(numFilters + 1) * i);
    }

    ArrayXd fftBinFreq(numFftBins);
    for (int i = 0; i < numFftBins; ++i) {
        fftBinFreq[i] = fs / 2.0 / static_cast<double>(numFftBins - 1) * i;
    }

    ArrayXXd fBank(numFilters, numFftBins);

    for (int filt = 1; filt <= numFilters; ++filt) {
        for (int bin = 0; bin < numFftBins; ++bin) {
            double weight;
            if (fftBinFreq[bin] < filterCentreFreq[filt - 1])
                weight = 0;
            else if (fftBinFreq[bin] <= filterCentreFreq[filt])
                weight = (fftBinFreq[bin] - filterCentreFreq[filt - 1]) / (filterCentreFreq[filt] - filterCentreFreq[filt - 1]);
            else if (fftBinFreq[bin] <= filterCentreFreq[filt + 1])
                weight = (filterCentreFreq[filt + 1] - fftBinFreq[bin]) / (filterCentreFreq[filt + 1] - filterCentreFreq[filt]);
            else
                weight = 0;
            fBank(filt - 1, bin) = weight;
        }
    }

    return fBank;
}

ArrayXd MFCC::applyDCT(const ArrayXXd & dct, const ArrayXd & lmfbCoef)
{
    int numCepstra = dct.rows() - 1;
    int numFilters = dct.cols();

    ArrayXd mfcc(numCepstra + 1);
    mfcc.setZero();
    for (int i = 0; i <= numCepstra; ++i) {
        for (int j = 0; j < numFilters; ++j) {
            mfcc(i) += dct(i, j) * lmfbCoef(j);
        }
    }

    return mfcc;
}

void MFCC::mfccFilter(const ArrayXd & poly, double fs,
                      int numCepstra, int numFilters,
                      double lowFreq, double highFreq,
                      ArrayXd & mfcc)
{
    int numFFT = fs <= 20'000 ? 512 : 2048;
    int numFftBins = numFFT / 2 + 1;

    // Initialize and memoize (if params unchanged) DCT and mel filter bank.

    static ArrayXXd dct;
    static ArrayXXd fBank;

    static struct {
        double fs;
        int numCep, numFlt, numFFT;
        double low, high;
    } last = {0, 0, 0, 0, 0};

    #pragma omp critical
    {
        if (last.numCep != numCepstra || last.numFlt != numFilters) {
            dct = initDCT(numCepstra, numFilters);
        }

        if (last.fs != fs || last.numFlt != numFilters || last.numFFT != numFFT
            || last.low != lowFreq || last.high != highFreq) {
            fBank = initFilterBank(lowFreq, highFreq, fs, numFilters, numFftBins);
        }

        last = {
                .fs = fs,
                .numCep = numCepstra,
                .numFlt = numFilters,
                .numFFT = numFFT,
                .low = lowFreq,
                .high = highFreq
        };
    }

    // END

    /* Step 1. compute power spectrum */
    ArrayXd f(numFftBins);
    for (int i = 0; i < numFftBins; ++i) {
        f(i) = (fs * static_cast<double>(i)) / static_cast<double>(numFFT);
    }
    ArrayXcd h;
    Filter::responseIIR(ArrayXd::Ones(1), poly, f, fs, h);

    ArrayXd powerSpectralCoef = h.abs().square();

    /* Step 2. apply log mel filterbank */
    ArrayXd lmfbCoef(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        for (int j = 0; j < numFftBins; ++j) {
            lmfbCoef[i] += fBank(i, j) * powerSpectralCoef(j);
        }
        if (lmfbCoef(i) < 1.0)
            lmfbCoef(i) = 1.0;
    }

    lmfbCoef = log(lmfbCoef);

    /* Step 3. apply discrete cosine transform */
    mfcc = applyDCT(dct, lmfbCoef);

}

void MFCC::mfccSignal(const ArrayXd & sound, double fs,
                      int numCepstra, int numFilters,
                      double lowFreq, double highFreq,
                      ArrayXd & mfcc)
{
    int numFFT = 2048;
    int numFftBins = numFFT / 2 + 1;

    // Initialize and memoize (if params unchanged) DCT and mel filter bank.

    static ArrayXXd dct;
    static ArrayXXd fBank;

    static struct {
        double fs;
        int numCep, numFlt, numFFT;
        double low, high;
    } last = {0, 0, 0, 0, 0};

#pragma omp critical
    {
        if (last.numCep != numCepstra || last.numFlt != numFilters) {
            dct = initDCT(numCepstra, numFilters);
        }

        if (last.fs != fs || last.numFlt != numFilters || last.numFFT != numFFT
            || last.low != lowFreq || last.high != highFreq) {
            fBank = initFilterBank(lowFreq, highFreq, fs, numFilters, numFftBins);
        }

        last = {
                .fs = fs,
                .numCep = numCepstra,
                .numFlt = numFilters,
                .numFFT = numFFT,
                .low = lowFreq,
                .high = highFreq
        };
    }

    // END

    /* Step 1. compute power spectrum */
    fft_plan(numFFT);
    Map<ArrayXcd>(fft_in(numFFT), numFFT).head(sound.size()) = sound;
    fft(numFFT);
    ArrayXd powerSpectralCoef = Map<ArrayXcd>(fft_out(numFFT), numFFT).head(numFftBins).abs().square();

    /* Step 2. apply log mel filterbank */
    ArrayXd lmfbCoef(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        for (int j = 0; j < numFftBins; ++j) {
            lmfbCoef[i] += fBank(i, j) * powerSpectralCoef(j);
        }
        if (lmfbCoef(i) < 1.0)
            lmfbCoef(i) = 1.0;
    }

    lmfbCoef = log(lmfbCoef);

    /* Step 3. apply discrete cosine transform */
    mfcc = applyDCT(dct, lmfbCoef);

}