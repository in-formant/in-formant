#ifndef TIME_TRACK_IMPLEMENTATION
#define TIME_TRACK_IMPLEMENTATION

template<typename T>
struct KeyComp {
    bool operator()(const double& a, const std::pair<double, T>& b) {
        return a < b.first;
    }
    bool operator()(const std::pair<double, T>& a, const double& b) {
        return a.first < b;
    }
};

template<typename T>
TimeTrack<T>::TimeTrack()
{
    mTrack.reserve(1000000);
}

template<typename T>
void TimeTrack<T>::insert(double t, const T& o)
{
    mTrack.emplace(upper_bound(t), t, o);
}

template<typename T>
typename TimeTrack<T>::iterator TimeTrack<T>::lower_bound(double t)
{
    return std::lower_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
typename TimeTrack<T>::iterator TimeTrack<T>::upper_bound(double t)
{
    return std::upper_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
typename TimeTrack<T>::const_iterator TimeTrack<T>::lower_bound(double t) const
{
    return std::lower_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
typename TimeTrack<T>::const_iterator TimeTrack<T>::upper_bound(double t) const
{
    return std::upper_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
const T& TimeTrack<T>::back() const
{
    return mTrack.back().second;
}

template<typename T>
bool TimeTrack<T>::empty() const
{
    return mTrack.empty();
}

#endif // TIME_TRACK_IMPLEMENTATION
