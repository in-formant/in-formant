#ifndef WAVELET_WT_H
#define WAVELET_WT_H

#include "common.h"
#include "convolution.h"
#include "wavelets.h"

#ifdef TYPE
#error TYPE should not be defined here.
#else

#ifdef REAL_TYPE
#error REAL_TYPE should not be defined here.
#else

#define TYPE double
#define REAL_TYPE double
#include "wt.template.h"
#undef REAL_TYPE
#undef TYPE

#include <complex>

using double_complex = std::complex<double>;

#define TYPE double_complex
#define REAL_TYPE double
#include "wt.template.h"
#undef REAL_TYPE
#undef TYPE

#endif /* REAL_TYPE */
#endif /* TYPE */

#endif // WAVELET_WT_H
