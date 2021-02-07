//
// Fast Fourier transform using PFFFT
//
// Copyright (C) 2017-2020 Andreas Gustafsson.  This file is part of
// the Gaborator library source distribution.  See the file LICENSE at
// the top level of the distribution for license information.
//

#ifndef _GABORATOR_FFT_PFFFT_H
#define _GABORATOR_FFT_PFFFT_H

#include <assert.h>
#include <complex>

#include <iterator>
#include <vector>


#include "pffft.h"

// XXX disable in production
#ifdef __x86_64__
#define GABORATOR_PFFFT_CHECK_ALIGN(p) assert((((uint64_t)(p)) & 0xF) == 0)
#else
#define GABORATOR_PFFFT_CHECK_ALIGN(p) do {} while (0)
#endif

namespace gaborator {

template <>
struct fft<std::complex<float> *> {
    typedef std::complex<float> *I;
    typedef const std::complex<float> *CONST_I;
    typedef std::iterator_traits<I>::value_type C; // complex
    typedef C::value_type T; // float/double

    fft(unsigned int n_): n(n_) {
        setup = pffft_new_setup(n, PFFFT_COMPLEX);
        assert(setup);
    }
    ~fft() {
        pffft_destroy_setup(setup);
    }

    unsigned int size() { return n; }

    // in-place
    void
    transform(I a) {
        pffft_transform_ordered(setup, (float *)a, (float *)a, NULL, PFFFT_FORWARD);
    }

    void
    itransform(I a) {
        pffft_transform_ordered(setup, (float *)a, (float *)a, NULL, PFFFT_BACKWARD);
    }

    // out-of-place
    void
    transform(CONST_I in, I out) {
        GABORATOR_PFFFT_CHECK_ALIGN(in);
        GABORATOR_PFFFT_CHECK_ALIGN(out);
        pffft_transform_ordered(setup, (const float *)in, (float *)out, NULL, PFFFT_FORWARD);
    }

    void
    itransform(CONST_I in, I out) {
        GABORATOR_PFFFT_CHECK_ALIGN(in);
        GABORATOR_PFFFT_CHECK_ALIGN(out);
        pffft_transform_ordered(setup, (const float *)in, (float *)out, NULL, PFFFT_BACKWARD);
    }

private:
    // Size of the transform
    unsigned int n;
    PFFFT_Setup *setup;
};

// Support transforming std::vector<std::complex<float> >::iterator

template <>
struct fft<std::vector<std::complex<float> >::iterator>:
    public fft<std::complex<float> *>
{
    typedef fft<std::complex<float> *> base;
    typedef std::vector<std::complex<float> >::iterator I;
    fft(unsigned int n_): fft<std::complex<float> *>(n_) { }
    void
    transform(I a) {
        base::transform(&(*a));
    }
    void
    itransform(I a) {
        base::itransform(&(*a));
    }
    void
    transform(I in, I out) {
        base::transform(&(*in), &(*out));
    }
    void
    itransform(I in, I out) {
        base::itransform(&(*in), &(*out));
    }
};

// Use fftpack for double precision

#define FFTPACK_DOUBLE_PRECISION 1
#include "fftpack.h"
#undef FFTPACK_DOUBLE_PRECISION

template <>
struct fft<std::complex<double> *> {
    typedef std::complex<double> *I;
    typedef const std::complex<double> *CONST_I;
    typedef std::iterator_traits<I>::value_type C; // complex
    typedef C::value_type T; // float/double

    fft(unsigned int n_): n(n_), wsave(4 * n_ + 15) {
        cffti(n, wsave.data());
    }
    ~fft() {
    }

    unsigned int size() { return n; }

    // in-place
    void
    transform(I a) {
        cfftf(n, (double *)a, wsave.data());
    }

    void
    itransform(I a) {
        cfftb(n, (double *)a, wsave.data());
    }

    // out-of-place
    void
    transform(CONST_I in, I out) {
        std::copy(in, in + n, out);
        transform(out);
    }

    void
    itransform(CONST_I in, I out) {
        std::copy(in, in + n, out);
        itransform(out);
    }

private:
    // Size of the transform
    unsigned int n;
    std::vector<double> wsave;
};

// Real FFT

template <>
struct rfft<std::complex<float> *> {
    typedef std::complex<float> *CI; // Complex iterator
    typedef const std::complex<float> *CONST_CI;
    typedef typename std::iterator_traits<CI>::value_type C; // complex
    typedef typename C::value_type T; // float/double
    typedef T *RI; // Real iterator
    typedef const T *CONST_RI;

    rfft(unsigned int n_): n(n_) {
        setup = pffft_new_setup(n, PFFFT_REAL);
        assert(setup);
    }
    ~rfft() {
        pffft_destroy_setup(setup);
    }

    unsigned int size() { return n; }

    // out-of-place only
    void
    transform(CONST_RI in, CI out) {
        GABORATOR_PFFFT_CHECK_ALIGN(in);
        GABORATOR_PFFFT_CHECK_ALIGN(out);
        pffft_transform_ordered(setup, in, (float *) out, NULL, PFFFT_FORWARD);
        C tmp = out[0];
#if GABORATOR_REAL_FFT_NEGATIVE_FQS
        for (unsigned int i = 1; i < (n >> 1); i++)
            out[n - i] = conj(out[i]);
#endif
        out[0] = C(tmp.real(), 0);
        out[n >> 1] = C(tmp.imag(), 0);
    }

    // Note: this temporarily modifies in[0], in spite of the const
    void
    itransform(CONST_CI in, RI out) {
        GABORATOR_PFFFT_CHECK_ALIGN(in);
        GABORATOR_PFFFT_CHECK_ALIGN(out);
        C tmp = in[0];
        const_cast<CI>(in)[0] = C(tmp.real(), in[n >> 1].real());
        pffft_transform_ordered(setup, (const float *) in, out, NULL, PFFFT_BACKWARD);
        const_cast<CI>(in)[0] = tmp;
    }

private:
    // Size of the transform
    unsigned int n;
    PFFFT_Setup *setup;
};

#undef GABORATOR_PFFFT_CHECK_ALIGN

} // namespace

#endif
