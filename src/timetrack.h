#ifndef TIME_TRACK_H
#define TIME_TRACK_H

#include "rpcxx.h"
#include <optional>
#include <algorithm>
#include <mutex>

template<typename T>
class TimeTrack {
public:
    using vector_type    = rpm::vector<std::pair<double, T>>;
    using iterator       = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;
    
    TimeTrack();
    TimeTrack(const TimeTrack&);

    void insert(double t, const T& o);
    void remove_before(double t);

    iterator lower_bound(double t);
    iterator upper_bound(double t);

    const_iterator lower_bound(double t) const;
    const_iterator upper_bound(double t) const;

    const T& back() const;

    bool empty() const;

private:
    vector_type mTrack;
};

template<typename T>
using OptionalTimeTrack = TimeTrack<std::optional<T>>;

#include "timetrack.ipp"

#endif // TIME_TRACK_H
