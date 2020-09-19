#ifndef ANALYSIS_FFT_H
#define ANALYSIS_FFT_H

#include <fftw3.h>
#include <complex>

#if defined(EMSCRIPTEN)
#   define FFTW_EM_FLAG FFTW_ESTIMATE
#else
#   define FFTW_EM_FLAG FFTW_MEASURE
#endif

namespace std
{
    using dcomplex = complex<double>;

    inline dcomplex& cast_dcomplex(fftw_complex& z)
    {
        return reinterpret_cast<dcomplex&>(z);
    }

    inline const dcomplex& cast_dcomplex(const fftw_complex& z)
    {
        return reinterpret_cast<const dcomplex&>(z);
    }
}

namespace Analysis
{
    void importFFTWisdom();

    class ReReFFT
    {
    public:
        ReReFFT(size_t n, fftw_r2r_kind method);
        ~ReReFFT();
        
        double data(int index) const;
        double& data(int index);

        void compute();

        size_t getLength() const;

    private:
        void checkIndex(int index) const;

        size_t mSize;
        fftw_plan mPlan;

        double *mData;
    };

    class RealFFT
    {
    public:
        RealFFT(size_t n);
        ~RealFFT();

        double input(int index) const;
        double& input(int index);

        std::dcomplex output(int index) const;
        std::dcomplex& output(int index);

        void computeForward();
        void computeBackward();

        size_t getInputLength() const;
        size_t getOutputLength() const;

    private:
        void checkInputIndex(int index) const;
        void checkOutputIndex(int index) const;

        size_t mSize;
        fftw_plan mPlanForward;
        fftw_plan mPlanBackward;

        double *mIn;
        fftw_complex *mOut;
    };

    class ComplexFFT
    {
    public:
        ComplexFFT(size_t n);
        ~ComplexFFT();

        std::dcomplex data(int index) const;
        std::dcomplex& data(int index);
        
        void computeForward();
        void computeBackward();

        size_t getLength() const;

    private:
        void checkIndex(int index) const;

        size_t mSize;
        fftw_plan mPlanForward;
        fftw_plan mPlanBackward;

        fftw_complex *mData;
    };
}

#endif // ANALYSIS_FFT_H
