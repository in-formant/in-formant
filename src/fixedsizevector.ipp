#ifndef FIXED_SIZE_VECTOR_IMPLEMENTATION
#define FIXED_SIZE_VECTOR_IMPLEMENTATION

#include <functional>
#include <stdexcept>

template<typename T>
FixedSizeVector<T>::FixedSizeVector(int size, const T &defaultValue)
    : mData(size, defaultValue),
      mDefaultValue(defaultValue),
      mSize(size),
      mStart(0)
{
}

template<typename T>
void FixedSizeVector<T>::push_back(const T &value)
{
    std::lock_guard<std::mutex> lock(mMutex);

    mStart = logicalIndexToPhysical(1);
    mData[logicalIndexToPhysical(-1)] = value;
}

template<typename T>
T FixedSizeVector<T>::back()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mSize == 0) {
        throw std::length_error("FixedSizeVector<T>::back() : vector is empty");
    }
    return mData[logicalIndexToPhysical(-1)];
}

template<typename T>
T FixedSizeVector<T>::operator[] (int index)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (index < -mSize || index >= mSize) {
        throw std::out_of_range("FixedSizeVector<T>::operator[]() : index is out of range");
    }
    return mData[logicalIndexToPhysical(index)];
}

template<typename T>
rpm::vector<T> FixedSizeVector<T>::tail(int count)
{
    rpm::vector<T> tail(count);
    for (int i = 0; i < count; ++i) {
        tail[i] = mData[logicalIndexToPhysical(-count - 1 + i)];
    }
    return tail;
}

template<typename T>
void FixedSizeVector<T>::resize(int size)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (size < 0) {
        throw std::invalid_argument("FixedSizeVector<T>::resize() : new size is negative");
    }

    rpm::vector<T> newData(size, mDefaultValue);

    if (mSize > size) {
        std::move(
                std::prev(mData.end(), size),
                mData.end(),
                newData.begin());
    }
    else {
        std::move(
                mData.begin(),
                mData.end(),
                std::prev(newData.end(), mSize));
    }

    mData = std::move(newData);
    mSize = size;
    mStart = 0;
}

template<typename T>
typename FixedSizeVector<T>::Iterator FixedSizeVector<T>::begin()
{
    return Iterator(*this, 0);
}

template<typename T>
typename FixedSizeVector<T>::Iterator FixedSizeVector<T>::end()
{
    return Iterator(*this, mSize);
}

template<typename T>
inline int FixedSizeVector<T>::logicalIndexToPhysical(int index)
{
    return (mSize + mStart + index) % mSize;
}

template<typename T>
FixedSizeVector<T>::Iterator::Iterator()
    : mContainer(nullptr), mPos(0)
{
}

template<typename T>
FixedSizeVector<T>::Iterator::Iterator(const FixedSizeVector<T>::Iterator& it)
    : mContainer(it.mContainer), mPos(it.mPos)
{
}

template<typename T>
FixedSizeVector<T>::Iterator::Iterator(FixedSizeVector<T>& container, int pos)
    : mContainer(&container), mPos(pos)
{
}

template<typename T>
typename FixedSizeVector<T>::Iterator& FixedSizeVector<T>::Iterator::operator++ ()
{
    ++mPos;
    return *this;
}

template<typename T>
typename FixedSizeVector<T>::Iterator FixedSizeVector<T>::Iterator::operator++ (int)
{
    Iterator tmp(*this);
    operator++ ();
    return tmp;
}

template<typename T>
typename FixedSizeVector<T>::Iterator& FixedSizeVector<T>::Iterator::operator-- ()
{
    --mPos;
    return *this;
}

template<typename T>
typename FixedSizeVector<T>::Iterator FixedSizeVector<T>::Iterator::operator-- (int)
{
    Iterator tmp(*this);
    operator-- ();
    return tmp;
}

template<typename T>
T& FixedSizeVector<T>::Iterator::operator* () const
{
    return mContainer->mData[mContainer->logicalIndexToPhysical(mPos)];
}

template<typename T>
bool FixedSizeVector<T>::Iterator::operator== (const Iterator& rhs) const
{
    return mContainer == rhs.mContainer && mPos == rhs.mPos;
}

template<typename T>
bool FixedSizeVector<T>::Iterator::operator!= (const Iterator& rhs) const
{
    return mContainer != rhs.mContainer || mPos != rhs.mPos;
}

#endif // FIXED_SIZE_VECTOR_IMPLEMENTATION

