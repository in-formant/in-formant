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
TimeTrack<T>::TimeTrack(const TimeTrack<T> &other)
    : mTrack(other.mTrack.begin(), other.mTrack.end())
{
}

template<typename T>
void TimeTrack<T>::insert(double t, const T& o)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mTrack.emplace(
        std::upper_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>()),
        t, o);
}

template<typename T>
void TimeTrack<T>::remove_before(double t)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mTrack.erase(mTrack.begin(), 
        std::upper_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>()));
}

template<typename T>
typename TimeTrack<T>::iterator TimeTrack<T>::lower_bound(double t)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return std::lower_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
typename TimeTrack<T>::iterator TimeTrack<T>::upper_bound(double t)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return std::upper_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
typename TimeTrack<T>::const_iterator TimeTrack<T>::lower_bound(double t) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return std::lower_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
typename TimeTrack<T>::const_iterator TimeTrack<T>::upper_bound(double t) const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return std::upper_bound(mTrack.begin(), mTrack.end(), t, KeyComp<T>());
}

template<typename T>
const T& TimeTrack<T>::back() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mTrack.back().second;
}

template<typename T>
bool TimeTrack<T>::empty() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mTrack.empty();
}

#endif // TIME_TRACK_IMPLEMENTATION
