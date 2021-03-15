#include "fft.h"
#include <stdexcept>

using namespace Analysis;

ComplexFFT::ComplexFFT(size_t n)
    : mSize(n),
      mData(fftw_alloc_complex(n))
{
    QMutexLocker lock(&sFFTWPlanMutex);
    importFFTWisdom();
    mPlanForward = fftw_plan_dft_1d(n, mData, mData, FFTW_FORWARD, FFTW_EM_FLAG);
    mPlanBackward = fftw_plan_dft_1d(n, mData, mData, FFTW_BACKWARD, FFTW_EM_FLAG);
}

ComplexFFT::~ComplexFFT()
{
    QMutexLocker lock(&sFFTWPlanMutex);
    fftw_destroy_plan(mPlanForward);
    fftw_destroy_plan(mPlanBackward);
    fftw_free(mData);
}

std::dcomplex ComplexFFT::data(int index) const
{
    checkIndex(index);
    return std::cast_dcomplex(mData[index]);
}

std::dcomplex& ComplexFFT::data(int index)
{
    checkIndex(index);
    return std::cast_dcomplex(mData[index]);
}

void ComplexFFT::computeForward()
{
    fftw_execute(mPlanForward);
}

void ComplexFFT::computeBackward()
{
    fftw_execute(mPlanBackward);
}

size_t ComplexFFT::getLength() const
{
    return mSize;
}

void ComplexFFT::checkIndex(int index) const
{
    if (index < 0 || index >= getLength()) {
        throw std::runtime_error("FFT::ComplexFFT] Data array index out of range");
    }
}
