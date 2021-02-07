#ifndef FIXED_SIZE_VECTOR_H
#define FIXED_SIZE_VECTOR_H

#include "rpcxx.h"
#include <mutex>

template<typename T>
class FixedSizeVectorIterator;

template<typename T>
class FixedSizeVector {
    class Iterator;

public:
    FixedSizeVector(int size = 0, const T& defaultValue = T());

    void push_back(const T& value);
    T back();
    T operator[] (int index);
    
    rpm::vector<T> tail(int count);

    void resize(int size);

    Iterator begin();
    Iterator end();

private:
    inline int logicalIndexToPhysical(int index);

    std::mutex mMutex;
    rpm::vector<T> mData;
    T mDefaultValue;
    int mSize;
    int mStart;
};

template<typename T>
class FixedSizeVector<T>::Iterator {
public:
    Iterator();
    Iterator(const FixedSizeVector<T>::Iterator&);
    Iterator& operator++ ();
    Iterator operator++ (int);
    Iterator& operator-- ();
    Iterator operator-- (int);
    T& operator* () const;
    bool operator== (const Iterator&) const;
    bool operator!= (const Iterator&) const;

private:
    Iterator(FixedSizeVector<T>& container, int pos);

    FixedSizeVector<T> *mContainer;
    int mPos;

    friend Iterator FixedSizeVector<T>::begin();
    friend Iterator FixedSizeVector<T>::end();
};

#include "fixedsizevector.ipp"

#endif // FIXED_SIZE_VECTOR_H
