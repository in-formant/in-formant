#ifndef ANALYSIS_FFT_H
#define ANALYSIS_FFT_H

#include <fftw3.h>
#include <complex>

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
    class RealFFT
    {
    public:
        RealFFT(size_t n);
        ~RealFFT();

        double input(int index) const;
        double& input(int index);
        std::dcomplex output(int index) const;
        std::dcomplex& output(int index);

        void compute();

        size_t getInputLength() const;
        size_t getOutputLength() const;

    private:
        void checkInputIndex(int index) const;
        void checkOutputIndex(int index) const;

        size_t mSize;
        fftw_plan mPlan;

        double *mIn;
        fftw_complex *mOut;
    };
}

#endif // ANALYSIS_FFT_H
