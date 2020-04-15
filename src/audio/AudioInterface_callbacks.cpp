//
// Created by clo on 13/09/2019.
//

#include <iostream>
#include "AudioInterface.h"

using namespace Eigen;

void AudioInterface::recordCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    auto context = static_cast<struct RecordContext *>(pDevice->pUserData);

    ArrayXXd deinterleavedInput = Map<const ArrayXXf>((float *) pInput, context->numChannels, frameCount).cast<double>();

    ArrayXd meanInput = deinterleavedInput.colwise().mean();

    context->buffer.writeInto(meanInput);
}

void AudioInterface::playCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    auto context = static_cast<PlaybackContext *>(pDevice->pUserData);
    auto output = static_cast<float *>(pOutput);
    
    const int numChannels = context->numChannels;

    for (int i = 0; i < signed(frameCount); ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            output[numChannels * i + ch] = 0.0;
        }
    }

    context->sineWave->readFrames(output, frameCount);
    context->noiseFilter->readFrames(output, frameCount);
}
