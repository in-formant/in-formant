#include "fft.h"
#include <stdexcept>

using namespace Analysis;

ReReFFT::ReReFFT(size_t n, fftw_r2r_kind method)
    : mSize(n),
      mData(fftw_alloc_real(n))
{
    importFFTWisdom();
    mPlan = fftw_plan_r2r_1d(n, mData, mData, method, FFTW_EM_FLAG);
}

ReReFFT::~ReReFFT()
{
    fftw_destroy_plan(mPlan);
    fftw_free(mData);
}

double ReReFFT::data(int index) const
{
    checkIndex(index);
    return mData[index];
}

double& ReReFFT::data(int index)
{
    checkIndex(index);
    return mData[index];
}

void ReReFFT::compute()
{
    fftw_execute(mPlan);
}

size_t ReReFFT::getLength() const
{
    return mSize;
}

void ReReFFT::checkIndex(int index) const
{
    if (index < 0 || index >= getLength()) {
        throw std::runtime_error("FFT::RealFFT] Data array index out of range");
    }
}

