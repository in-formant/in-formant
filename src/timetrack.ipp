#ifndef TIME_TRACK_IMPLEMENTATION
#define TIME_TRACK_IMPLEMENTATION

template<typename T>
void TimeTrack<T>::insert(double t, const T& o)
{
    mTrack.emplace(t, o);
}

template<typename T>
typename TimeTrack<T>::iterator TimeTrack<T>::lower_bound(double t)
{
    return mTrack.lower_bound(t);
}

template<typename T>
typename TimeTrack<T>::iterator TimeTrack<T>::upper_bound(double t)
{
    return mTrack.upper_bound(t);
}

template<typename T>
typename TimeTrack<T>::const_iterator TimeTrack<T>::lower_bound(double t) const
{
    return mTrack.lower_bound(t);
}

template<typename T>
typename TimeTrack<T>::const_iterator TimeTrack<T>::upper_bound(double t) const
{
    return mTrack.upper_bound(t);
}

template<typename T>
const T& TimeTrack<T>::back() const
{
    return std::prev(mTrack.end())->second;
}

template<typename T>
bool TimeTrack<T>::empty() const
{
    return mTrack.empty();
}

#endif // TIME_TRACK_IMPLEMENTATION
