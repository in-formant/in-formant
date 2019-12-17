//
// Created by clo on 18/11/2019.
//

#ifndef SPEECH_ANALYSIS_FFT_H
#define SPEECH_ANALYSIS_FFT_H

#include <Eigen/Core>
#include <fftw3.h>

template<typename T1, typename T2>
class fft_s {
public:
    int n;
    T1 * in;
    T2 * out;
    fftw_plan plan;

public:
    fft_s() : n(-1), plan(nullptr), in(nullptr), out(nullptr) {}
    fft_s(int n)
            : n(n), in((T1 *) fftw_malloc(n * sizeof(T1))),
              out((T2 *) fftw_malloc(n * sizeof(T2))) {
    }
    ~fft_s() {
        if (n > 0) {
            if (plan != nullptr) fftw_destroy_plan(plan);
            if (in != nullptr) fftw_free(in);
            if (out != nullptr) fftw_free(out);
        }
    }

    fft_s & operator=(const fft_s & other) noexcept {
        this->~fft_s();
        n = other.n;
        plan = other.plan;
        in = other.in;
        out = other.out;
        return *this;
    }

    fft_s & operator=(fft_s && other) noexcept {
        *this = (const fft_s &) other;
        other.n = -1;
        other.plan = nullptr;
        other.in = nullptr;
        other.out = nullptr;
        return *this;
    }
};

// 0.5 seconds
#define FFT_PLAN_TIMELIMIT (0.5)

#define DECL_FFT(name, inType, outType) \
    inType * name##_in(int n); \
    outType * name##_out(int n); \
    void name##_plan(int n); \
    void name(int n);

DECL_FFT(rfft, double, double)
DECL_FFT(irfft, double, double)
DECL_FFT(rcfft, double, Eigen::dcomplex)
DECL_FFT(crfft, Eigen::dcomplex, double)
DECL_FFT(fft, Eigen::dcomplex, Eigen::dcomplex)
DECL_FFT(ifft, Eigen::dcomplex, Eigen::dcomplex)

void all_fft_cleanup();

#endif //SPEECH_ANALYSIS_FFT_H
