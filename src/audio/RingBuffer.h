//
// Created by rika on 11/10/2019.
//

#ifndef SPEECH_ANALYSIS_RINGBUFFER_H
#define SPEECH_ANALYSIS_RINGBUFFER_H

#include <Eigen/Core>
#include <mutex>
#include <vector>

class RingBuffer {
public:
    explicit RingBuffer(int capacity = 0);

    void writeInto(const Eigen::ArrayXd & in);
    void readFrom(Eigen::ArrayXd & out);

    void setCapacity(int newCapacity);

private:
    int capacity, writeCursor;
    std::vector<double> data;
    std::mutex mut;
};

template<typename T>
void swapEndian(T * value)
{
    static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = *value;

    for (size_t ib = 0; ib < sizeof(T); ++ib) {
        dest.u8[ib] = source.u8[sizeof(T) - ib - 1];
    }

    *value = dest.u;
}

#endif //SPEECH_ANALYSIS_RINGBUFFER_H
