cmake_minimum_required(VERSION 3.5)
project(libsamplerate)

set(CMAKE_AUTOMOC OFF)

set(PACKAGE "libsamplerate")
set(VERSION "0.1.9")
set(PACKAGE_STRING "${PACKAGE} ${VERSION}")
set(PACKAGE_BUGREPORT "erikd@mega-nerd.com")
set(PACKAGE_TARNAME "libsamplerate")
set(PACKAGE_URL "http://www.mega-nerd.com/libsamplerate/")
set(PACKAGE_NAME "${PACKAGE}")
set(PACKAGE_VERSION "${VERSION}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_IS_GCC 1)
else()
    set(COMPILER_IS_GCC 0)
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test-clip-positive.c [[
#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1
#define	__USE_ISOC99	1
#define __USE_ISOC9X	1
#include <math.h>
int main (void)
{	double	fval ;
    int k, ival ;
    fval = 1.0 * 0x7FFFFFFF ;
    for (k = 0 ; k < 100 ; k++)
    {
        ival = (lrint (fval)) >> 24 ;
        if (ival != 127)
            return 1 ;
    
        fval *= 1.2499999 ;
    } ;

    return 0 ;
}
]])

try_run(CPU_CLIPS_POSITIVE _compileVar ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/test-clip-positive.c)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test-clip-negative.c [[
#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1
#define	__USE_ISOC99	1
#define __USE_ISOC9X	1
#include <math.h>
int main (void)
{	double	fval ;
    int k, ival ;
    fval = -8.0 * 0x10000000 ;
    for (k = 0 ; k < 100 ; k++)
    {
        ival = (lrint (fval)) >> 24 ;
        if (ival != -128)
            return 1 ;
    
        fval *= 1.2499999 ;
    } ;

    return 0 ;
}
]])

try_run(CPU_CLIPS_NEGATIVE _compileVar ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/test-clip-negative.c)

if(CMAKE_C_BYTE_ORDER STREQUAL BIG_ENDIAN)
    set(CPU_IS_BIG_ENDIAN 1)
    set(CPU_IS_LITTLE_ENDIAN 0)
else()
    set(CPU_IS_BIG_ENDIAN 0)
    set(CPU_IS_LITTLE_ENDIAN 1)
endif()

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckTypeSize)

check_library_exists(m pow "" HAVE_LIBM)
if(HAVE_LIBM)
    set (CMAKE_REQUIRED_LIBRARIES "m;${CMAKE_REQUIRED_LIBRARIES}")
endif()
check_function_exists(alert HAVE_ALARM)
check_include_file(alsa/asoundlib.h HAVE_ALSA_ASOUNDLIB_H)
check_function_exists(calloc HAVE_CALLOC)
check_function_exists(ceil HAVE_CEIL)
check_include_file(dlfcn.h HAVE_DLFCN_H)
set(HAVE_FFTW3 1)
check_function_exists(floor HAVE_FLOOR)
check_function_exists(fmod HAVE_FMOD)
check_function_exists(free HAVE_FREE)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_function_exists(lrint HAVE_LRINT)
check_function_exists(lrintf HAVE_LRINTF)
check_function_exists(malloc HAVE_MALLOC)
check_function_exists(memcpy HAVE_MEMCPY)
check_function_exists(memmove HAVE_MEMMOVE)
check_include_file(memory.h HAVE_MEMORY_H)
if (NOT MSVC)
    set(HAVE_SIGALRM 1)
    set(HAVE_SIGNAL 1)
else()
    set(HAVE_SIGALRM 0)
    set(HAVE_SIGNAL 0)
endif()
set(HAVE_SNDFILE 0)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/times.h HAVE_SYS_TIMES_H)
check_include_file(unistd.h HAVE_UNISTD_H)
set(LT_OBJDIR ".libs/")
if(WIN32)
    set(OS_IS_WIN32 1)
else()
    set(OS_IS_WIN32 0)
endif()
check_type_size(double SIZEOF_DOUBLE)
check_type_size(float SIZEOF_FLOAT)
check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)
check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

configure_file(src/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions(DLL_EXPORT)

add_library(lsr SHARED
    src/common.h
    src/fastest_coeffs.h
    src/float_cast.h
    src/high_qual_coeffs.h
    src/mid_qual_coeffs.h
    src/samplerate.c
    src/samplerate.h
    src/src_linear.c
    src/src_sinc.c
    src/src_zoh.c)
