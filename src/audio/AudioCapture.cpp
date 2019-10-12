//
// Created by clo on 12/09/2019.
//

#include <iostream>
#include "AudioCapture.h"
#include "../Exceptions.h"

static constexpr std::array<SoundIoFormat, 4> prioritisedFormats = {
        SoundIoFormatFloat64NE,
        SoundIoFormatFloat32NE,
        SoundIoFormatFloat64FE,
        SoundIoFormatFloat32FE,
};

static constexpr std::array<int, 5> prioritisedSampleRates = {
        32000,
        22050,
        44100,
        48000,
};

template<typename T>
static void convertEndian(T array[], int length);

AudioCapture::AudioCapture() {

    SDL_zero(audioContext);

    int ret;

    soundio = soundio_create();
    if (soundio == nullptr) {
        throw SioException("Unable to create instance", "out of memory");
    }

    ret = soundio_connect(soundio);
    if (ret != 0) {
        throw SioException("Unable to connect to a backend", ret);
    }

    soundio_flush_events(soundio);

    // Select default input device.
    selectCaptureDevice();

    // Open and start input stream.
    openInputStream();

}

AudioCapture::~AudioCapture() {

    soundio_instream_destroy(inputStream);
    soundio_device_unref(inputDevice);
    soundio_destroy(soundio);

}

void AudioCapture::selectCaptureDevice(int index) {

    if (index < 0) {
        // Select default input device.
        index = soundio_default_input_device_index(soundio);
    } else if (index >= soundio_input_device_count(soundio)) {
        throw SioException("Unable to select input device", "index out of range");
    }

    // Retrieve device description.
    inputDevice = soundio_get_input_device(soundio, index);
    if (inputDevice == nullptr) {
        throw SioException("Unable to select input device", "no input devices available");
    }

    std::cout << "Selected input device: " << inputDevice->name << std::endl;

    if (inputDevice->probe_error != 0) {
        throw SioException("Unable to probe device", inputDevice->probe_error);
    }

    soundio_device_sort_channel_layouts(inputDevice);

    // Find preferred channel layout.
    const struct SoundIoChannelLayout *preferredLayout;
    const struct SoundIoChannelLayout *layout;

    preferredLayout = soundio_channel_layout_get_default(1);
    layout = soundio_best_matching_channel_layout(preferredLayout, 1,
                                                  inputDevice->layouts, inputDevice->layout_count);

    // Find preferred sample rate.
    int sampleRate = 0;

    for (int preferredSampleRate : prioritisedSampleRates) {
        if (soundio_device_supports_sample_rate(inputDevice, preferredSampleRate)) {
            sampleRate = preferredSampleRate;
            break;
        }
    }

    if (sampleRate == 0) {
        sampleRate = inputDevice->sample_rates[0].max;
    }

    // Find preferred format.
    enum SoundIoFormat format = SoundIoFormatInvalid;

    for (auto preferredFormat : prioritisedFormats) {
        if (soundio_device_supports_format(inputDevice, preferredFormat)) {
            format = preferredFormat;
            break;
        }
    }

    if (format == SoundIoFormatInvalid) {
        throw SioException("Unable to create input stream", "floating point format unsupported");
    }

    // Create input stream.
    inputStream = soundio_instream_create(inputDevice);
    if (inputStream == nullptr) {
        throw SioException("Unable to create input stream", "out of memory");
    }

    inputStream->layout = *layout;
    inputStream->format = format;
    inputStream->sample_rate = sampleRate;
    inputStream->read_callback = &readCallback;
    inputStream->overflow_callback = &AudioCapture::overflowCallback;
    inputStream->userdata = &audioContext;
}

void AudioCapture::openInputStream() {

    int ret;

    // Open the input stream.

    ret = soundio_instream_open(inputStream);
    if (ret != 0) {
        throw SioException("Unable to open input stream", ret);
    }

    std::cout << inputStream->layout.name << " "
              << inputStream->sample_rate << "Hz "
              << soundio_format_string(inputStream->format) << " interleaved" << std::endl;

    // Create the ring buffer.
    audioContext.buffer.setCapacity(BUFFER_SAMPLE_COUNT(inputStream->sample_rate));

    // Start the input stream.

    ret = soundio_instream_start(inputStream);
    if (ret != 0) {
        throw SioException("Unable to start input stream", ret);
    }

}

int AudioCapture::getSampleRate() const noexcept {
    return inputStream->sample_rate;
}

void AudioCapture::readBlock(Eigen::ArrayXd & capture) noexcept {
    capture.resize(CAPTURE_SAMPLE_COUNT(inputStream->sample_rate));
    audioContext.buffer.readFrom(capture);
}

template<typename T>
void convertEndian(T array[], int length) {

    static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    for (int k = 0; k < length; ++k) {
        source.u = array[k];

        for (size_t ib = 0; ib < sizeof(T); ++ib) {
            dest.u8[ib] = source.u8[sizeof(T) - ib - 1];
        }

        array[k] = dest.u;
    }

}