//
// Fast Fourier transform
//
// Copyright (C) 2016-2020 Andreas Gustafsson.  This file is part of
// the Gaborator library source distribution.  See the file LICENSE at
// the top level of the distribution for license information.
//

#ifndef _GABORATOR_FFT_H
#define _GABORATOR_FFT_H

#include "gaborator/fft_naive.h"

#if GABORATOR_USE_VDSP
#include "gaborator/fft_vdsp.h"
#define GABORATOR_USE_REAL_FFT 1
#elif GABORATOR_USE_PFFFT
#include "gaborator/fft_pffft.h"
#define GABORATOR_USE_REAL_FFT 1
#else
// Use the naive FFT
// Do not define GABORATOR_USE_REAL_FFT as it is slower than
// using the complex code.
#endif

#endif
