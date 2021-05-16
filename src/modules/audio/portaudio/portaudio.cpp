#include "portaudio.h"
#include <iostream>

using namespace Module::Audio;

PortAudio::PortAudio()
    : mCaptureStream(nullptr),
      mPlaybackStream(nullptr)
{
}

PortAudio::~PortAudio()
{
}

void PortAudio::initialize()
{
    err = Pa_Initialize();
    checkError();
}

void PortAudio::terminate()
{
    err = Pa_Terminate();
    checkError();
}

void PortAudio::refreshDevices()
{
    mCaptureDevices.clear();
    mPlaybackDevices.clear();

    PaDeviceIndex deviceCount = Pa_GetDeviceCount();
    if (deviceCount < 0) {
        throw std::runtime_error(std::string("Audio::PortAudio] ") + Pa_GetErrorText(deviceCount));
    }

    PaHostApiIndex defaultHostApi;
#if defined(__linux__)
    defaultHostApi = Pa_HostApiTypeIdToHostApiIndex(paALSA);
#elif defined(_WIN32)
    defaultHostApi = Pa_HostApiTypeIdToHostApiIndex(paMME);
#elif defined(__APPLE__)
    defaultHostApi = Pa_HostApiTypeIdToHostApiIndex(paCoreAudio);
#else
    defaultHostApi = Pa_GetDefaultHostApi();
#endif

    const PaHostApiInfo *hostApiInfo = Pa_GetHostApiInfo(defaultHostApi);

    PaDeviceIndex defaultCaptureDevice = hostApiInfo->defaultInputDevice;
    if (defaultCaptureDevice == paNoDevice) {
        throw std::runtime_error("Audio::PortAudio] No default input device found");
    }

    PaDeviceIndex defaultPlaybackDevice = hostApiInfo->defaultOutputDevice;
    if (defaultPlaybackDevice == paNoDevice) {
        throw std::runtime_error("Audio::PortAudio] No default output device found");
    }

    for (PaDeviceIndex i = 0; i < deviceCount; ++i) {
        auto deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo == nullptr) {
            throw std::runtime_error("Audio::PortAudio] Device index out of range");
        }

        Device device(Backend::PortAudio);

        device.portaudio.index = i;
        device.portaudio.inputLatency = deviceInfo->defaultHighInputLatency;
        device.portaudio.outputLatency = deviceInfo->defaultHighOutputLatency;
        device.portaudio.sampleRate = deviceInfo->defaultSampleRate;
        device.name = deviceInfo->name;

        if (deviceInfo->maxInputChannels > 0) {
            if (i == defaultCaptureDevice) {
                mDefaultCaptureDeviceIndex = mCaptureDevices.size();
            }
            mCaptureDevices.push_back(device);
        }

        if (deviceInfo->maxOutputChannels > 0) {
            if (i == defaultPlaybackDevice) {
                mDefaultPlaybackDeviceIndex = mPlaybackDevices.size();
            }
            mPlaybackDevices.push_back(device);
        }
    }
}

const rpm::vector<Device>& PortAudio::getCaptureDevices() const
{
    return mCaptureDevices;
}

const rpm::vector<Device>& PortAudio::getPlaybackDevices() const
{
    return mPlaybackDevices;
}

const Device& PortAudio::getDefaultCaptureDevice() const
{
    return mCaptureDevices[mDefaultCaptureDeviceIndex];
}

const Device& PortAudio::getDefaultPlaybackDevice() const
{
    return mPlaybackDevices[mDefaultPlaybackDeviceIndex];
}

void PortAudio::openCaptureStream(const Device *pDevice)
{
    if (pDevice == nullptr) {
        pDevice = &mCaptureDevices[mDefaultCaptureDeviceIndex];
    }

    PaStreamParameters params;
    params.channelCount = 1;
    params.device = pDevice->portaudio.index;
    params.hostApiSpecificStreamInfo = nullptr;
    params.sampleFormat = paFloat32;
    params.suggestedLatency = pDevice->portaudio.inputLatency; 

    err = Pa_OpenStream(
            &mCaptureStream,
            &params,
            nullptr,
            pDevice->portaudio.sampleRate,
            1024,
            paNoFlag,
            &PortAudio::captureCallback,
            this);
    checkError();

    mPauseCapture = true;
    setCaptureBufferSampleRate(pDevice->portaudio.sampleRate);
}

void PortAudio::startCaptureStream()
{
    err = Pa_IsStreamStopped(mCaptureStream);
    if (err < 0) {
        checkError();
    }
    if (!err) {
        std::cout << "Audio::PortAudio] Stream is not stopped" << std::endl;
        return;
    }

    mPauseCapture = false;

    err = Pa_StartStream(mCaptureStream);
    checkError();
}

void PortAudio::stopCaptureStream()
{
    err = Pa_IsStreamStopped(mCaptureStream);
    if (err < 0) {
        checkError();
    }
    if (err) {
        std::cout << "Audio::PortAudio] Stream is stopped" << std::endl;
        return;
    }

    mPauseCapture = true;

    err = Pa_StopStream(mCaptureStream);
    checkError();
}

void PortAudio::closeCaptureStream()
{
    err = Pa_CloseStream(mCaptureStream);
    checkError();
    mCaptureStream = nullptr;
}

void PortAudio::openPlaybackStream(const Device *pDevice)
{
    if (pDevice == nullptr) {
        pDevice = &mPlaybackDevices[mDefaultPlaybackDeviceIndex];
    }

    PaStreamParameters params;
    params.channelCount = 1;
    params.device = pDevice->portaudio.index;
    params.hostApiSpecificStreamInfo = nullptr;
    params.sampleFormat = paFloat32;
    params.suggestedLatency = pDevice->portaudio.outputLatency;
    
    err = Pa_OpenStream(
            &mPlaybackStream,
            nullptr,
            &params,
            pDevice->portaudio.sampleRate,
            256,
            paNoFlag,
            &PortAudio::playbackCallback,
            this);
    checkError();

    mPausePlayback = true;
    mPlaybackQueue->setOutSampleRate(pDevice->portaudio.sampleRate);
}

void PortAudio::startPlaybackStream()
{
    err = Pa_IsStreamStopped(mPlaybackStream);
    if (err < 0) {
        checkError();
    }
    if (!err) {
        std::cout << "Audio::PortAudio] Stream is not stopped" << std::endl;
        return;
    }

    mPausePlayback = false;

    err = Pa_StartStream(mPlaybackStream);
    checkError();
}

void PortAudio::stopPlaybackStream()
{
    err = Pa_IsStreamStopped(mPlaybackStream);
    if (err < 0) {
        checkError();
    }
    if (err) {
        std::cout << "Audio::PortAudio] Stream is stopped" << std::endl;
        return;
    }

    mPausePlayback = true;

    err = Pa_StopStream(mPlaybackStream);
    checkError();
}

void PortAudio::closePlaybackStream()
{
    err = Pa_CloseStream(mPlaybackStream);
    checkError();
}

int PortAudio::captureCallback(
        const void *input,
        void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData)
{
    auto that = static_cast<PortAudio *>(userData);

    that->pushToCaptureBuffer(static_cast<const float *>(input), frameCount);

    return that->mPauseCapture ? paComplete : paContinue;
}

#include <cmath>

int PortAudio::playbackCallback(
        const void *input,
        void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData)
{
    auto that = static_cast<PortAudio *>(userData);

    that->mPlaybackQueue->pull(static_cast<float *>(output), frameCount);

    return that->mPausePlayback ? paComplete : paContinue;
}

void PortAudio::checkError()
{
    if (err != paNoError) {
        throw std::runtime_error(std::string("Audio::PortAudio] ") + Pa_GetErrorText(err));
    }
}


