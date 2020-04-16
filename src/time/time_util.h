#pragma once

#include "datatypes.h"

// sleep for msec
void SleepInMs(uint32 ms);

// sleep for usec
void SleepInUs(uint32 us);

inline void SleepInSeconds(uint32 sec) {
    SleepInMs(sec * 1000);
}

// monotonic timestamp in usec
uint64 NowInUs();

// monotonic timestamp in msec
inline uint64 NowInMs() {
    return NowInUs() / 1000;
}

class WallTimer {
  public:
    WallTimer() {
        _start = NowInUs();
    }
    ~WallTimer() {
    }

    void Restart() {
        _start = NowInUs();
    }

    uint64 ElapseInUs() const {
        return NowInUs() - _start;
    }

    uint64 ElapseInMs() const {
        return this->ElapseInUs() / 1000;
    }

  private:
    uint64 _start;

    DISALLOW_COPY_AND_ASSIGN(WallTimer);
};
