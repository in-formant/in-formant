#ifndef WAVELET_CONVOLUTION_H
#define WAVELET_CONVOLUTION_H

#ifdef TYPE
#error TYPE should not be defined here.
#else

#ifdef REAL_TYPE
#error REAL_TYPE should not be defined here.
#else

#include "common.h"

#define TYPE double
#define REAL_TYPE double
#include "convolution.template.h"
#undef REAL_TYPE
#undef TYPE

#include <complex>

using double_complex = std::complex<double>;

#define TYPE double_complex
#define REAL_TYPE double
#include "convolution.template.h"
#undef REAL_TYPE
#undef TYPE

#endif /* REAL_TYPE */
#endif /* TYPE */

#endif // WAVELET_CONVOLUTION_H
