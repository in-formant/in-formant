//
// Created by rika on 11/10/2019.
//

#include "RingBuffer.h"

using namespace Eigen;

RingBuffer::RingBuffer(int capacity)
    : capacity(capacity), writeCursor(0)
{
    data.resize(capacity, 0.0);
}

void RingBuffer::writeInto(const ArrayXd & in)
{
    std::lock_guard<std::mutex> lock(mut);

    const int toWrite = in.size();
    const int tailCount = capacity - writeCursor;

    auto dataIt = data.begin() + writeCursor;

    if (toWrite <= tailCount) {
        std::copy_n(in.begin(), toWrite, dataIt);
    }
    else {
        auto inputIt = in.begin() + tailCount;

        std::copy_n(in.begin(), tailCount, dataIt);
        std::copy_n(inputIt, toWrite - tailCount, data.begin());
    }

    writeCursor = (writeCursor + toWrite) % capacity;
}

void RingBuffer::readFrom(ArrayXd & out)
{
    std::lock_guard<std::mutex> lock(mut);

    const int toRead = out.size();
    const int toReadRoundedUp = (1 + (toRead / capacity)) * capacity;

    const int readCursor = (toReadRoundedUp + writeCursor - toRead) % capacity;
    const int tailCount = capacity - readCursor;

    auto dataIt = data.begin() + readCursor;

    if (toRead <= tailCount) {
        std::copy_n(dataIt, toRead, out.begin());
    }
    else {
        auto outputIt = out.begin() + tailCount;

        std::copy_n(dataIt, tailCount, out.begin());
        std::copy_n(data.begin(), toRead - tailCount, outputIt);
    }
}

void RingBuffer::setCapacity(int newCapacity)
{
    std::lock_guard<std::mutex> lock(mut);

    capacity = newCapacity;
    data.resize(capacity, 0.0);
    writeCursor = 0;
}