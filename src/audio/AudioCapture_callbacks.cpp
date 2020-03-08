//
// Created by clo on 13/09/2019.
//

#include "AudioCapture.h"

using namespace Eigen;

void AudioCapture::readCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    auto context = (struct RecordContext *) pDevice->pUserData;

    ArrayXd block = Map<const ArrayXf>((float *) pInput, frameCount).cast<double>();

    context->buffer.writeInto(block);
}
