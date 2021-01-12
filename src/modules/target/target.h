#ifndef MODULES_TARGET_H
#define MODULES_TARGET_H

#include "base/base.h"

#ifdef TARGET_USE_SDL2
#   include "sdl2/sdl2.h"
#endif

#ifdef TARGET_USE_QT5QUICK
#   include "qt5quick/qt5quick.h"
#endif

#endif // MODULES_TARGET_H
