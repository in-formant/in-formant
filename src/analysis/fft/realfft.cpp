#include "fft.h"
#include <stdexcept>

using namespace Analysis;

RealFFT::RealFFT(size_t n)
    : mSize(n),
      mIn(fftw_alloc_real(n)),
      mOut(fftw_alloc_complex(n / 2 + 1))
{
    mPlan = fftw_plan_dft_r2c_1d(n, mIn, mOut, FFTW_MEASURE);
}

RealFFT::~RealFFT()
{
    fftw_destroy_plan(mPlan);
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

void RealFFT::compute()
{
    fftw_execute(mPlan);
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
