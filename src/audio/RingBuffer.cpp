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

    if (toWrite <= tailCount) {
        for (int i = 0; i < toWrite; ++i) {
            data[writeCursor + i] = in(i);
        }
    }
    else {
        for (int i = 0; i < tailCount; ++i) {
            data[writeCursor + i] = in(i);
        }
        for (int i = 0; i < toWrite - tailCount; ++i) {
            data[i] = in(tailCount + i);
        }
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

    if (toRead <= tailCount) {
        for (int i = 0; i < toRead; ++i) {
            out(i) = data[readCursor + i];
        }
    }
    else {
        for (int i = 0; i < tailCount; ++i) {
            out(i) = data[readCursor + i];
        }
        for (int i = 0; i < toRead - tailCount; ++i) {
            out(tailCount + i) = data[i];
        }
    }
}

void RingBuffer::setCapacity(int newCapacity)
{
    std::lock_guard<std::mutex> lock(mut);

    capacity = newCapacity;
    data.resize(capacity, 0.0);
    writeCursor = 0;
}
