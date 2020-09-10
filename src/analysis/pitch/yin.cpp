#include "pitch.h"

using Analysis::PitchResult;
using namespace Analysis::Pitch;

Yin::Yin(float threshold)
    : mThreshold(threshold),
      mFFT(nullptr)
{
}

PitchResult Yin::solve(const float *data, int length, int sampleRate) 
{
    mFFT.reset(new ComplexFFT(length));

    for (int i = 0; i < length; ++i) {
        mFFT->data(i) = data[i];
    }
    mFFT->computeForward();
    for (int i = 0; i < length; ++i) {
        std::dcomplex z = mFFT->data(i);
        mFFT->data(i) = (z * conj(z)) / (double) length;
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
        return {.pitch = 0.0f, .voiced = false};
    }
    else {
        float pitch = (float) sampleRate / (float) k;
        return {.pitch = pitch, .voiced = true};
    }
}

