//
// Created by clo on 13/09/2019.
//

#include "../Exceptions.h"
#include "AudioCapture.h"
#include <iostream>

void AudioCapture::readCallback(struct SoundIoInStream *instream, int frame_count_min, int frame_count_max) {

    auto ctx = static_cast<RecordContext *>(instream->userdata);

    struct SoundIoChannelArea *areas;
    int ret;

    char *writePtr = soundio_ring_buffer_write_ptr(ctx->buffer);

    const int freeBytes = soundio_ring_buffer_free_count(ctx->buffer);
    const int freeCount = freeBytes / instream->bytes_per_frame;

    if (freeCount < frame_count_min) {
        throw SioException("Read callback", "ring buffer overflow");
    }

    const int writeFrames = std::min(freeCount, frame_count_max);
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
            memset(writePtr, 0, frameCount * instream->bytes_per_frame);
        } else {
            for (int frame = 0; frame < frameCount; ++frame) {
                for (int ch = 0; ch < instream->layout.channel_count; ++ch) {
                    memcpy(writePtr, areas[ch].ptr, instream->bytes_per_sample);
                    areas[ch].ptr += areas[ch].step;
                    writePtr += instream->bytes_per_sample;
                }
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

    int advanceBytes = writeFrames * instream->bytes_per_frame;
    soundio_ring_buffer_advance_write_ptr(ctx->buffer, advanceBytes);

}

void AudioCapture::overflowCallback(struct SoundIoInStream *instream) {
    static int count = 0;

    std::cerr << "Overflow " << ++count << std::endl;
}