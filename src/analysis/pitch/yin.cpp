#include "pitch.h"

using Analysis::PitchResult;
using namespace Analysis::Pitch;

inline int
pow2roundup (int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

Yin::Yin(double threshold)
    : mThreshold(threshold),
      mFFT(nullptr)
{
}

PitchResult Yin::solve(const double *data, int length, int sampleRate) 
{
    int nfft = pow2roundup(length);

    if (!mFFT || mFFT->getLength() != nfft) {
        mFFT = std::make_shared<ComplexFFT>(nfft);
    }

    for (int i = 0; i < length; ++i) {
        mFFT->data(i) = data[i];
    }
    for (int i = length; i < nfft; ++i) {
        mFFT->data(i) = 0.0;
    }
    mFFT->computeForward();
    for (int i = 0; i < nfft; ++i) {
        std::dcomplex z = mFFT->data(i);
        mFFT->data(i) = (z * conj(z)) / (double) nfft;
    }
    mFFT->computeBackward();

    mAutocorrelation.resize(length);
    for (int i = 0; i < length; ++i) {
        mAutocorrelation[i] = mFFT->data(i).real();
    }

    mDifference.resize(length / 2);
    for (int tau = 0; tau < length / 2; ++tau) {
        mDifference[tau] = mAutocorrelation[0] + mAutocorrelation[1] - 2 * mAutocorrelation[tau];
    }

    mCMND.resize(length / 2);
    double runningSum = 0.0;
    mCMND[0] = 1.0;
    for (int tau = 1; tau < length / 2; ++tau) {
        runningSum += mDifference[tau];
        mCMND[tau] = (tau * mDifference[tau]) / runningSum;
    }

    int k;
    for (k = 2; k < length / 2; ++k) {
        if (mCMND[k] < mThreshold) {
            while (k + 1 < length / 2 && mCMND[k + 1] < mCMND[k])
                k++;
            break;
        }
    }

    if (k == length / 2 || mCMND[k] >= mThreshold) {
        return {.pitch = 0.0, .voiced = false};
    }
    else {
        double pitch = (double) sampleRate / (double) k;
        return {.pitch = pitch, .voiced = true};
    }
}

