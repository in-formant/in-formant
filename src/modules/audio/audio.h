#ifndef MODULES_AUDIO_H
#define MODULES_AUDIO_H

#include "base/base.h"
#include "resampler/resampler.h"
#include "buffer/buffer.h"
#include "queue/queue.h"

#ifdef AUDIO_USE_DUMMY
#   include "dummy/dummy.h"
#endif

#ifdef AUDIO_USE_ALSA
#   include "alsa/alsa.h"
#endif

#ifdef AUDIO_USE_PULSE
#   include "pulse/pulse.h"
#endif

#ifdef AUDIO_USE_PORTAUDIO
#   include "portaudio/portaudio.h"
#endif

#ifdef AUDIO_USE_OBOE
#   include "oboe/oboe.h"
#endif

#endif // MODULES_AUDIO_H
