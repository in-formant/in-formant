//
// Fast Fourier transform, naive reference implementations
//
// Copyright (C) 1992-2020 Andreas Gustafsson.  This file is part of
// the Gaborator library source distribution.  See the file LICENSE at
// the top level of the distribution for license information.
//

// Based on the module "fft" used in audsl/test, audsl/mls,
// scope/core, whitesig

#ifndef _GABORATOR_FFT_NAIVE_H
#define _GABORATOR_FFT_NAIVE_H

#include <algorithm>
#include <complex>
#include <iterator>
#include <vector>

#include <memory.h>

namespace gaborator {

template <class I>
struct fft {
    typedef typename std::iterator_traits<I>::value_type C; // complex
    typedef typename C::value_type T; // float/double
    typedef typename std::vector<C> twiddle_vector;

    fft(unsigned int n): n_(n), wtab(n / 2) { init_wtab(); }
    ~fft() { }

    unsigned int size() { return n_; }

    // Transform the contents of the array "a", leaving results in
    // bit-reversed order.
    void
    br_transform(I a) {
            unsigned int i, j, m, n;
            typename twiddle_vector::iterator wp; // twiddle factor pointer
            I p, q;

            // n is the number of points in each subtransform (butterfly group)
            // m is the number of subtransforms (butterfly groups), = n_ / n
            // i is the index of the first point in the current butterfly group
            // j is the number of the butterfly within the group

            for (n = 2, m = n_ / 2; n <= n_; n *= 2 , m /= 2) // each stage
                for (i = 0; i < n_; i += n)  // each butterfly group
                    for (j = 0, wp = wtab.begin(), p = a + i, q = a + i + n / 2;
                         j < n / 2;
                         j++, wp += m, p++, q++)    // each butterfly
                    {
                        C temp((*q) * (*wp));
                        *q = *p - temp;
                        *p += temp;
                    }
    }

    void
    bit_reverse(I a) {
        unsigned int i, j;
        for (i = 0, j = 0; i < n_; i++, j = bitrev_inc(j)) {
            if (i < j)
                std::swap(*(a + i), *(a + j));
        }
    }

    void
    reverse(I a) {
        for (unsigned int i = 1; i < n_ / 2; i++)
            std::swap(*(a + i), *(a + n_ - i));
    }

    // in-place
    void
    transform(I a) {
        bit_reverse(a);
        br_transform(a);
    }

    void
    itransform(I a) {
        reverse(a);
        transform(a);
    }

    // out-of-place
    // XXX const
    void
    transform(I in, I out) {
        std::copy(in, in + n_, out);
        transform(out);
    }

    void
    itransform(I in, I out) {
        std::copy(in, in + n_, out);
        itransform(out);
    }

private:
    // Initialize twiddle factor array
    void init_wtab() {
        size_t wt_size = wtab.size();
        for (size_t i = 0; i < wt_size; ++i) {
            double arg = (-2.0 * M_PI / n_) * i;
            wtab[i] = C(cos(arg), sin(arg));
        }
    }

    unsigned int
    bitrev_inc(unsigned int i) {
        unsigned int carry = n_;
        do {
            carry >>= 1;
            unsigned int new_i = i ^ carry;
            carry &= i;
            i = new_i;
        } while(carry);
        return i;
    }

    // Size of the transform
    unsigned int n_;

    // Twiddle factor array (size n / 2)
    twiddle_vector wtab;
};

// Real FFT
//
// This is a trivial implementation offering no performance advantage
// over a complex FFT.  It is intended as a placeholder to be
// overridden with a specialization, and as a reference implementation
// to compare the results of specializations against.
//

template <class CI>
struct rfft {
    typedef typename std::iterator_traits<CI>::value_type C; // complex
    typedef typename C::value_type T; // float/double
    typedef T *RI; // Real iterator
    typedef const T *CONST_RI;

    rfft(unsigned int n): cf(n) { }
    ~rfft() { }

    void
    transform(CONST_RI in, CI out) {
        size_t n = cf.size();
        C *tmp = new C[n];
        C *out_tmp = new C[n];
        std::copy(in, in + cf.size(), tmp); // Real to complex
        cf.transform(tmp, out_tmp);
        delete [] tmp;
#if GABORATOR_REAL_FFT_NEGATIVE_FQS
        std::copy(out_tmp, out_tmp + n, out);
#else
        std::copy(out_tmp, out_tmp + n / 2 + 1, out);
#endif
        delete [] out_tmp;
    }

    void
    itransform(CI in, RI out) {
        size_t n = cf.size();
        // Make sure not to use the negative frequency part of "in",
        // because it may not be valid.
        C *in_tmp = new C[n];
        for (size_t i = 0; i < n / 2 + 1; i++) {
            in_tmp[i] = in[i];
        }
        for (size_t i = 1; i < n / 2; i++) {
            in_tmp[n - i] = conj(in[i]);
        }
        C *tmp = new C[n];
        cf.itransform(in_tmp, tmp);
        for (size_t i = 0; i < n; i++) {
            *out++ = tmp[i].real();
        }
        delete [] tmp;
        delete [] in_tmp;
    }

    fft<CI> cf;
};

} // Namespace

#endif
