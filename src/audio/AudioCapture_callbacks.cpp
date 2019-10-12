//
// Created by clo on 13/09/2019.
//

#include "../Exceptions.h"
#include "AudioCapture.h"
#include <iostream>

using namespace Eigen;

void AudioCapture::readCallback(struct SoundIoInStream *instream, int frame_count_min, int frame_count_max) {

    auto ctx = static_cast<RecordContext *>(instream->userdata);

    struct SoundIoChannelArea *areas;
    int ret;

    const int writeFrames = frame_count_max;

    ArrayXd block(writeFrames);
    int blockIndex = 0;

    const bool isDoublePrecision = (instream->format == SoundIoFormatFloat64NE)
                                    || (instream->format == SoundIoFormatFloat64FE);

    const bool isNativeEndian = (instream->format == SoundIoFormatFloat32NE)
                                || (instream->format == SoundIoFormatFloat64NE);

    int framesLeft = writeFrames;
    while (true) {
        int frameCount = framesLeft;

        ret = soundio_instream_begin_read(instream, &areas, &frameCount);
        if (ret != 0) {
            throw SioException("Unable to begin read", ret);
        }

        if (frameCount == 0) {
            break;
        }

        if (areas == nullptr) {
            //overflow; fill the ring buffer with silence.
            block.setZero();
        } else {
            // For each frame...
            for (int frame = 0; frame < frameCount; ++frame) {
                // Average the values of each channel...
                double sum = 0.0;

                for (int ch = 0; ch < instream->layout.channel_count; ++ch) {
                    if (isDoublePrecision) {
                        double y;

                        memcpy(&y, areas[ch].ptr, instream->bytes_per_sample);
                        areas[ch].ptr += areas[ch].step;

                        if (!isNativeEndian) {
                            swapEndian(&y);
                        }

                        sum += y;
                    } else {
                        float y;

                        memcpy(&y, areas[ch].ptr, instream->bytes_per_sample);
                        areas[ch].ptr += areas[ch].step;

                        if (!isNativeEndian) {
                            swapEndian(&y);
                        }

                        sum += y;
                    }
                }

                sum /= static_cast<double>(instream->layout.channel_count);

                block(blockIndex++) = sum;
            }
        }

        ret = soundio_instream_end_read(instream);
        if (ret != 0) {
            throw SioException("Unable to end read", ret);
        }

        framesLeft -= frameCount;
        if (framesLeft <= 0) {
            break;
        }
    }

    ctx->buffer.writeInto(block);

}

void AudioCapture::overflowCallback(struct SoundIoInStream *instream) {
    static int count = 0;

    std::cerr << "Overflow " << ++count << std::endl;
}