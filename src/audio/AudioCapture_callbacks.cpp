//
// Created by clo on 13/09/2019.
//

#include "AudioCapture.h"
#include <iostream>

using namespace Eigen;

template<typename T>
static void convertToFloat(ArrayXd & dst, T * src, unsigned long len, T offset, T max);

int AudioCapture::readCallback(const void * input, void * output,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo * timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void * userData)
{
    auto ctx = static_cast<RecordContext *>(userData);

    ArrayXd block;

    if (ctx->format == paFloat32) {
        auto inPtr = static_cast<const float *>(input);
        block = Map<const ArrayXf>(inPtr, frameCount).cast<double>();
    }
    else if (ctx->format == paInt32) {
        auto inPtr = static_cast<const int32_t *>(input);
        convertToFloat<const int32_t>(block, inPtr, frameCount, 0, INT32_MAX);
    }
    else if (ctx->format == paInt16) {
        auto inPtr = static_cast<const int16_t *>(input);
        convertToFloat<const int16_t>(block, inPtr, frameCount, 0, INT16_MAX);
    }
    else if (ctx->format == paInt8) {
        auto inPtr = static_cast<const int8_t *>(input);
        convertToFloat<const int8_t>(block, inPtr, frameCount, 0, INT8_MAX);
    }
    else if (ctx->format == paUInt8) {
        auto inPtr = static_cast<const uint8_t *>(input);
        convertToFloat<const uint8_t>(block, inPtr, frameCount, 128, INT8_MAX);
    }

    ctx->buffer.writeInto(block);

    return paContinue;

}

template<typename T>
void convertToFloat(ArrayXd & dst, T * src, unsigned long len, T offset, T max)
{
    for (int i = 0; i < len; ++i) {
        dst(i) = static_cast<double>(offset + src[i]) / static_cast<double>(max);
    }
}
