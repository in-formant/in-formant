//
// Created by clo on 18/11/2019.
//

#include "FFT.h"
#include <map>

using Eigen::dcomplex;

#define DECL_IN(name, type, s_var) type * name##_in(int n) { return s_var.find(n)->second.in; }
#define DECL_OUT(name, type, s_var) type * name##_out(int n) { return s_var.find(n)->second.out; }

#define DECL_PLAN(name, s_var, plan_call) \
    void name##_plan(int n) { \
        fftw_set_timelimit(FFT_PLAN_TIMELIMIT); \
        auto it = s_var.find(n); \
        if (it == s_var.end()) { \
             auto s = decltype(it->second)(n); \
             s.plan = plan_call; \
             s_var[n] = std::move(s); \
        } \
    }

#define DECL_EXEC(name, s_var) \
    void name(int n) { \
        fftw_execute(s_var.find(n)->second.plan); \
    }

#define DECL_FFT_IMPL(name, inType, outType, plan_call) \
    static std::map<int, fft_s<inType, outType>> s_##name; \
    DECL_IN(name, inType, s_##name) \
    DECL_OUT(name, outType, s_##name) \
    DECL_PLAN(name, s_##name, plan_call) \
    DECL_EXEC(name, s_##name)

DECL_FFT_IMPL(rfft, double, double, fftw_plan_r2r_1d(n, s.in, s.out, FFTW_REDFT10, FFTW_MEASURE))
DECL_FFT_IMPL(irfft, double, double, fftw_plan_r2r_1d(n, s.in, s.out, FFTW_REDFT01, FFTW_MEASURE))

DECL_FFT_IMPL(rcfft, double, dcomplex, fftw_plan_dft_r2c_1d(n, s.in, (fftw_complex *) s.out, FFTW_MEASURE))
DECL_FFT_IMPL(crfft, dcomplex, double, fftw_plan_dft_c2r_1d(n, (fftw_complex *) s.in, s.out, FFTW_MEASURE))

DECL_FFT_IMPL(fft, dcomplex, dcomplex, fftw_plan_dft_1d(n, (fftw_complex *) s.in, (fftw_complex *) s.out, FFTW_FORWARD, FFTW_MEASURE))
DECL_FFT_IMPL(ifft, dcomplex, dcomplex, fftw_plan_dft_1d(n, (fftw_complex *) s.in, (fftw_complex *) s.out, FFTW_BACKWARD, FFTW_MEASURE))

void all_fft_cleanup()
{
    s_rfft.clear();
    s_irfft.clear();
    s_rcfft.clear();
    s_crfft.clear();
    s_fft.clear();
    s_ifft.clear();
    fftw_cleanup();
}