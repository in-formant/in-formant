#include "pulse.h"
#include <iostream>
#include <memory>

using namespace Module::Audio;

Pulse::Pulse()
    : mDefaultSource(Backend::Pulse),
      mDefaultSink(Backend::Pulse)
{
}

Pulse::~Pulse()
{
}

void Pulse::initialize()
{
    mThreadedMl = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(mThreadedMl);
    mMlApi = pa_threaded_mainloop_get_api(mThreadedMl);
    mContext = pa_context_new(mMlApi, "in-formant");

    pa_context_connect(mContext, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    pa_usec_t start = pa_rtclock_now();
    do {
        pa_context_state_t contextState = pa_context_get_state(mContext);
        if (contextState == PA_CONTEXT_READY) {
            break;
        }
        else if (contextState == PA_CONTEXT_FAILED) {
            throw std::runtime_error("Audio::Pulse] The connection failed or was disconnected");
        }
        else if (contextState == PA_CONTEXT_TERMINATED) {
            throw std::runtime_error("Audio::Pulse] The connection was terminated");
        }
        else if (pa_rtclock_now() - start >= apiTimeoutUsec) {
            throw std::runtime_error("Audio::Pulse] The connection attempt timed out");
        }
        pa_msleep(msleepDur);
    } while (true);
}

void Pulse::terminate()
{
    pa_context_disconnect(mContext);
    pa_context_unref(mContext);
    pa_threaded_mainloop_stop(mThreadedMl);
    pa_threaded_mainloop_free(mThreadedMl);
}

void Pulse::refreshDevices()
{
    mSources.clear();
    mSinks.clear();

    struct DevArg {
        rpm::vector<Device> *list;
        bool end;
    };

    DevArg sourceArg { .list = &mSources, .end = false };
    DevArg sinkArg { .list = &mSinks, .end = false };

    auto opSources = pa_context_get_source_info_list(
            mContext,
            [](auto, const pa_source_info *i, int eol, void *userdata) {
                auto arg = static_cast<DevArg *>(userdata);
                if (eol) {
                    arg->end = true;
                    return;
                }
                Device dev(Backend::Pulse);
                dev.name = i->name;
                arg->list->push_back(dev);
            },
            &sourceArg);
    
    pa_operation_unref(opSources);

    auto opSinks = pa_context_get_sink_info_list(
            mContext,
            [](auto, const pa_sink_info *i, int eol, void *userdata) {
                auto arg = static_cast<DevArg *>(userdata);
                if (eol) {
                    arg->end = true;
                    return;
                }
                Device dev(Backend::Pulse);
                dev.name = i->name;
                arg->list->push_back(dev);
            },
            &sinkArg);
    
    pa_operation_unref(opSinks);

    struct {
        bool found;
        std::string sinkName;
        std::string sourceName;
    } defArg { false, "", "" };

    auto opServer = pa_context_get_server_info(
            mContext,
            [](auto, const pa_server_info *i, void *userdata) {
                auto arg = static_cast<std::add_pointer_t<decltype(defArg)>>(userdata);
                arg->found = true;
                arg->sinkName = i->default_sink_name;
                arg->sourceName = i->default_source_name;
            },
            &defArg);

    pa_operation_unref(opServer);

    pa_usec_t start = pa_rtclock_now();
    while (!sourceArg.end && !sinkArg.end && !defArg.found) {
        if (pa_rtclock_now() - start >= apiTimeoutUsec) {
            throw std::runtime_error("Audio::Pulse] The device enumeration attempt timed out");
        }
        pa_msleep(msleepDur);
    }

    mDefaultSource.name = defArg.sourceName;
    mDefaultSink.name = defArg.sinkName;
}

const rpm::vector<Device>& Pulse::getCaptureDevices() const
{
    return mSources;
}

const rpm::vector<Device>& Pulse::getPlaybackDevices() const
{
    return mSinks;
}

const Device& Pulse::getDefaultCaptureDevice() const
{
    return mDefaultSource;
}

const Device& Pulse::getDefaultPlaybackDevice() const
{
    return mDefaultSink;
}

void Pulse::openCaptureStream(const Device *pDevice)
{
    if (pDevice == nullptr) {
        pDevice = &getDefaultCaptureDevice();
    }

    pa_sample_spec ss { .format = PA_SAMPLE_FLOAT32NE, .rate = 48000, .channels = 1 };
    pa_channel_map map { .channels = 1, .map = { PA_CHANNEL_POSITION_MONO } };

    mCaptureStream = pa_stream_new(mContext, "in-formant capture", &ss, &map);

    pa_stream_set_read_callback(
            mCaptureStream,
            [](pa_stream *p, size_t, void *userdata) {
                auto that = static_cast<Pulse *>(userdata);

                const void *data;
                size_t nbytes;
                pa_stream_peek(p, &data, &nbytes);

                if (data == nullptr && nbytes == 0) {
                    return;
                }

                int length = nbytes / sizeof(float);

                if (data == nullptr) { 
                    auto array = std::make_unique<float[]>(length);
                    std::fill(array.get(), std::next(array.get(), length), 0.0f);
                    that->pushToCaptureBuffer(array.get(), length);
                }
                else {
                    that->pushToCaptureBuffer(static_cast<const float *>(data), length);
                }

                pa_stream_drop(p);
            },
            this);

    pa_stream_connect_record(mCaptureStream, pDevice->name.c_str(), nullptr, (pa_stream_flags_t) (PA_STREAM_FIX_RATE | PA_STREAM_START_CORKED));

    pa_usec_t start = pa_rtclock_now();
    do {
        pa_stream_state_t streamState = pa_stream_get_state(mCaptureStream);
        if (streamState == PA_STREAM_READY) {
            break;
        }
        else if (streamState == PA_STREAM_FAILED) {
            throw std::runtime_error("Audio::Pulse] The capture stream connection failed or was disconnected");
        }
        else if (streamState == PA_STREAM_TERMINATED) {
            throw std::runtime_error("Audio::Pulse] The capture stream connection was terminated");
        }
        else if (pa_rtclock_now() - start >= apiTimeoutUsec) {
            throw std::runtime_error("Audio::Pulse] The capture stream connection attempt timed out");
        }
        pa_msleep(msleepDur);
    } while (true);

    setCaptureBufferSampleRate(pa_stream_get_sample_spec(mCaptureStream)->rate);
}

void Pulse::startCaptureStream()
{
    pa_stream_cork(mCaptureStream, false, nullptr, nullptr);
}

void Pulse::stopCaptureStream()
{
    pa_stream_cork(mCaptureStream, true, nullptr, nullptr);
}

void Pulse::closeCaptureStream()
{
    pa_stream_disconnect(mCaptureStream);
    pa_stream_unref(mCaptureStream);
}

void Pulse::openPlaybackStream(const Device *pDevice)
{
    if (pDevice == nullptr) {
        pDevice = &getDefaultPlaybackDevice();
    }

    pa_sample_spec ss { .format = PA_SAMPLE_FLOAT32NE, .rate = 48000, .channels = 1 };
    pa_channel_map map { .channels = 1, .map = { PA_CHANNEL_POSITION_MONO } };

    mPlaybackStream = pa_stream_new(mContext, "in-formant playback", &ss, &map);

    pa_stream_set_write_callback(
            mPlaybackStream,
            [](pa_stream *p, size_t nbytes, void *userdata) {
                auto queue = static_cast<Queue *>(userdata);

                int length = nbytes / sizeof(float);

                auto data = new float[length];
                queue->pull(data, length);
                
                pa_stream_write(p, data, nbytes, [](void *p) { delete[] static_cast<float *>(p); }, 0, PA_SEEK_RELATIVE);
            },
            mPlaybackQueue);

    pa_stream_connect_playback(mPlaybackStream, pDevice->name.c_str(), nullptr, (pa_stream_flags_t) (PA_STREAM_FIX_RATE | PA_STREAM_START_CORKED), nullptr, nullptr);

    pa_usec_t start = pa_rtclock_now();
    do {
        pa_stream_state_t streamState = pa_stream_get_state(mPlaybackStream);
        if (streamState == PA_STREAM_READY) {
            break;
        }
        else if (streamState == PA_STREAM_FAILED) {
            throw std::runtime_error("Audio::Pulse] The playback stream connection failed or was disconnected");
        }
        else if (streamState == PA_STREAM_TERMINATED) {
            throw std::runtime_error("Audio::Pulse] The playback stream connection was terminated");
        }
        else if (pa_rtclock_now() - start >= apiTimeoutUsec) {
            throw std::runtime_error("Audio::Pulse] The playback stream connection attempt timed out");
        }
        pa_msleep(msleepDur);
    } while (true);

    mPlaybackQueue->setOutSampleRate(pa_stream_get_sample_spec(mPlaybackStream)->rate);
}

void Pulse::startPlaybackStream()
{
    pa_stream_cork(mPlaybackStream, false, nullptr, nullptr);
}

void Pulse::stopPlaybackStream()
{
    pa_stream_cork(mPlaybackStream, true, nullptr, nullptr);
}

void Pulse::closePlaybackStream()
{
    pa_stream_disconnect(mPlaybackStream);
    pa_stream_unref(mPlaybackStream);
}

void Pulse::checkError()
{
}

