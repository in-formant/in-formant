#define MINIAUDIO_SPEEX_RESAMPLER_IMPLEMENTATION
#include "speex/ma_speex_resampler.h"

#ifndef __EMSCRIPTEN__
#include "rpmalloc/rpmalloc.h"
#define USE_RPMALLOC
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

