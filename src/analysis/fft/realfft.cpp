#include "fft.h"
#include <stdexcept>

using namespace Analysis;

std::mutex Analysis::sFFTWPlanMutex;

RealFFT::RealFFT(size_t n)
    : mSize(n),
      mIn(fftw_alloc_real(n)),
      mOut(fftw_alloc_complex(n / 2 + 1))
{
    std::lock_guard<std::mutex> lock(sFFTWPlanMutex);
    importFFTWisdom();
    mPlanForward = fftw_plan_dft_r2c_1d(n, mIn, mOut, FFTW_EM_FLAG);
    mPlanBackward = fftw_plan_dft_c2r_1d(n, mOut, mIn, FFTW_EM_FLAG);
}

RealFFT::~RealFFT()
{
    std::lock_guard<std::mutex> lock(sFFTWPlanMutex);
    fftw_destroy_plan(mPlanForward);
    fftw_destroy_plan(mPlanBackward);
    fftw_free(mIn);
    fftw_free(mOut);
}

double RealFFT::input(int index) const
{
    checkInputIndex(index);
    return mIn[index];
}

double& RealFFT::input(int index)
{
    checkInputIndex(index);
    return mIn[index];
}

std::dcomplex RealFFT::output(int index) const
{
    checkOutputIndex(index);
    return std::cast_dcomplex(mOut[index]);
}

std::dcomplex& RealFFT::output(int index)
{
    checkOutputIndex(index);
    return std::cast_dcomplex(mOut[index]);
}

void RealFFT::computeForward()
{
    fftw_execute(mPlanForward);
}

void RealFFT::computeBackward()
{
    fftw_execute(mPlanBackward);
}

size_t RealFFT::getInputLength() const
{
    return mSize;
}

size_t RealFFT::getOutputLength() const
{
    return mSize / 2 + 1;
}

void RealFFT::checkInputIndex(int index) const
{
    if (index < 0 || index >= getInputLength()) {
        throw std::runtime_error("FFT::RealFFT] Input array index out of range");
    }
}

void RealFFT::checkOutputIndex(int index) const
{
    if (index < 0 || index >= getOutputLength()) {
        throw std::runtime_error("FFT::RealFFT] Output array index out of range");
    }
}
