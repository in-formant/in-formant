#include "nodeio_frequencies.h"

using namespace Nodes::IO;

Frequencies::Frequencies()
    : mLength(0)
{
}

void Frequencies::setLength(int length)
{
    if (length == mLength) {
        return;
    }

    mData.resize(length);
    mLength = length;
}

void Frequencies::set(int index, float frequency)
{
    mData.at(index) = frequency;
}

int Frequencies::getLength() const
{
    return mLength;
}

const float Frequencies::get(int index) const
{
    return mData.at(index);
}

