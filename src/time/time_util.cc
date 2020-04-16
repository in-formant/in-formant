#include "time_util.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#else
#  include <time.h>
#  include <errno.h>

#  ifdef __APPLE__
#    include <mach/clock.h>
#    include <mach/mach.h>
#  endif
#endif // _WIN32

/**********************************=> unix ************************************/
#ifndef _WIN32
void SleepInMs(uint32 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = ms % 1000 * 1000000;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

void SleepInUs(uint32 us) {
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = us % 1000000 * 1000;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

#ifndef __APPLE__
uint64 NowInUs() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return static_cast<uint64>(now.tv_sec) * 1000000 + now.tv_nsec / 1000;
}

#else // mac
uint64 NowInUs() {
    clock_serv_t cs;
    mach_timespec_t ts;

    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cs);
    clock_get_time(cs, &ts);
    mach_port_deallocate(mach_task_self(), cs);

    return static_cast<uint64>(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
}
#endif // __APPLE__
#endif // _WIN32
/************************************ unix <=**********************************/

/**********************************=> win *************************************/
#ifdef _WIN32
void SleepInMs(uint32 ms) {
    ::Sleep(ms);
}

void SleepInUs(uint32 us) {
    ::LARGE_INTEGER ft;
    ft.QuadPart = -static_cast<int64>(us * 10);  // '-' using relative time

    ::HANDLE timer = ::CreateWaitableTimer(NULL, TRUE, NULL);
    ::SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    ::WaitForSingleObject(timer, INFINITE);
    ::CloseHandle(timer);
}

static inline uint64 GetPerfFrequency() {
    ::LARGE_INTEGER freq;
    ::QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

static inline uint64 PerfFrequency() {
    static uint64 xFreq = GetPerfFrequency();
    return xFreq;
}

static inline uint64 PerfCounter() {
    ::LARGE_INTEGER counter;
    ::QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

uint64 NowInUs() {
    return static_cast<uint64>(
        static_cast<double>(PerfCounter()) * 1000000 / PerfFrequency());
}
#endif // _WIN32
