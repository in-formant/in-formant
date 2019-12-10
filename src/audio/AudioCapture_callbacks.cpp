//
// Created by clo on 13/09/2019.
//

#include "AudioCapture.h"
#include <iostream>

using namespace Eigen;

template<typename T, T min, T max>
static void convertToFloat(ArrayXd & dst, T * src, unsigned long len);

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
        convertToFloat<const int32_t, 0, INT32_MAX>(block, inPtr, frameCount);
    }
    else if (ctx->format == paInt16) {
        auto inPtr = static_cast<const int16_t *>(input);
        convertToFloat<const int16_t, 0, INT16_MAX>(block, inPtr, frameCount);
    }
    else if (ctx->format == paInt8) {
        auto inPtr = static_cast<const int8_t *>(input);
        convertToFloat<const int8_t, 0, INT8_MAX>(block, inPtr, frameCount);
    }
    else if (ctx->format == paUInt8) {
        auto inPtr = static_cast<const uint8_t *>(input);
        convertToFloat<const uint8_t, 128, INT8_MAX>(block, inPtr, frameCount);
    }

    ctx->buffer.writeInto(block);

    return paContinue;

}

template<typename T, T offset, T max>
void convertToFloat(ArrayXd & dst, T * src, unsigned long len)
{
    for (int i = 0; i < len; ++i) {
        dst(i) = static_cast<double>(offset + src[i]) / static_cast<double>(max);
    }
}
