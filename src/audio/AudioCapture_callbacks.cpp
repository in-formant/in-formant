//
// Created by clo on 13/09/2019.
//

#include <iostream>
#include "AudioCapture.h"

using namespace Eigen;

void AudioCapture::readCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    if (pInput != nullptr) {
        auto context = (struct RecordContext *) pDevice->pUserData;

        ArrayXXd deinterleavedInput = Map<const ArrayXXf>((float *) pInput, context->numChannels, frameCount).cast<double>();

        ArrayXd meanInput = deinterleavedInput.colwise().mean();

        context->buffer.writeInto(meanInput);
    }

    if (pOutput != nullptr) {
        Map<ArrayXf>((float *) pOutput, frameCount).setZero();
    }
}
