#ifndef CONTEXT_TIMINGS_H
#define CONTEXT_TIMINGS_H

#include <QMutex>
#include <QMutexLocker>
#include <chrono>
#include <ostream>

using dmilli = std::chrono::duration<double, std::milli>;
using hr_clock = std::chrono::high_resolution_clock;
using time_point = hr_clock::time_point;

struct duration : public dmilli, public QMutex
{
    duration() : dmilli(0), mSet(false) {}
    constexpr operator double() { return count(); }
    constexpr duration& operator=(const dmilli& dur) {
        if (!mSet) {
            dmilli::operator=(dur);
            mSet = true;
        }
        else {
            dmilli::operator=(0.5 * (*this) + 0.5 * dur);
        }
        return *this;
    }
private:
    bool mSet;
};

struct timer_guard {
    timer_guard(duration& dur)
        : mLocker(&dur), mStart(hr_clock::now()), mDur(dur)
    {} 
    ~timer_guard() {
        mDur = (hr_clock::now() - mStart);
    }
    constexpr operator bool() {
        // Used for syntactic sugar.
        // RAII in an if-assignment statement.
        return true;
    }
private:
    QMutexLocker mLocker;
    time_point mStart;
    duration& mDur;
};

std::ostream& operator<<(std::ostream&, const duration&);

namespace timings {
    extern duration render;
    extern duration update;
}

#endif // CONTEXT_TIMINGS_H
