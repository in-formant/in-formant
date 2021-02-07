//
// The Gaussian and related functions
//
// Copyright (C) 2015-2018 Andreas Gustafsson.  This file is part of
// the Gaborator library source distribution.  See the file LICENSE at
// the top level of the distribution for license information.
//

#ifndef _GABORATOR_GAUSSIAN_H
#define _GABORATOR_GAUSSIAN_H

#include <math.h>

namespace gaborator {

// A rough approximation of erfc_inv(), the inverse complementary
// error function.  This is good for arguments in the range 1e-8 to 1,
// to within a few percent.

inline float erfc_inv(float x) {
    return sqrtf(-logf(x)) - 0.3f;
}

// Gaussian with peak = 1

inline double norm_gaussian(double sd, double x) {
    return exp(-(x * x) / (2 * sd * sd));
}

// Gaussian with integral = 1

inline double gaussian(double sd, double x) {
    double a = 1.0 / (sd * sqrt(2.0 * M_PI));
    return a * norm_gaussian(sd, x);
}

// The convolution of a Heaviside step function with a Gaussian of
// standard deviation sd.  Goes smoothly from 0 to 1, with the 0.5
// point at x=0.

static inline
double gaussian_edge(double sd, double x) {
    double erf_arg = x / (sd * M_SQRT2);
    if (erf_arg < -7)
        return 0; // error < 5e-23
    if (erf_arg > 7)
        return 1; // error < 5e-23
    return (erf(erf_arg) + 1) * 0.5;
}

// Translate the time-domain standard deviation of a gaussian
// (in samples) into the corresponding frequency-domain standard
// deviation (as a fractional frequency), or vice versa.

static inline double sd_t2f(double st_sd) {
    return 1.0 / (2.0 * M_PI * st_sd);
}

static inline double sd_f2t(double ff_sd) {
    return sd_t2f(ff_sd);
}

// Given a gaussian with standard deviation "sd" and a maximum error
// "max_error", calculate the support needed on each side to keep the
// area below the curve within max_error of the exact value.

static inline double gaussian_support(double sd, double max_error) {
    return sd * M_SQRT2 * erfc_inv(max_error);
}

// Inverse of the above: given a support and maximum error, calculate
// the standard deviation.

static inline double gaussian_support_inv(double support, double max_error) {
    return support / (M_SQRT2 * erfc_inv(max_error));
}

} // namespace

#endif
