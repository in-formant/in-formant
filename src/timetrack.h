#ifndef TIME_TRACK_H
#define TIME_TRACK_H

#include "rpcxx.h"
#include <optional>

template<typename T>
class TimeTrack {
public:
    using map_type       = rpm::map<double, T>;
    using iterator       = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

    void insert(double t, const T& o);

    iterator lower_bound(double t);
    iterator upper_bound(double t);

    const_iterator lower_bound(double t) const;
    const_iterator upper_bound(double t) const;

    const T& back() const;

    bool empty() const;

private:
    map_type mTrack;
};

template<typename T>
using OptionalTimeTrack = TimeTrack<std::optional<T>>;

#include "timetrack.ipp"

#endif // TIME_TRACK_H
