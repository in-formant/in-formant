#include "oboe.h"
#include <iostream>

#include <SDL2/SDL.h>
#include <jni.h>

using namespace Module::Audio;

Oboe::Oboe()
    : mCaptureStream(nullptr),
      mPlaybackStream(nullptr)
{
}

Oboe::~Oboe()
{
}

void Oboe::initialize()
{
    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    jobject activity = (jobject) SDL_AndroidGetActivity();

    jclass clazz = env->GetObjectClass(activity);
    jmethodID methodId = env->GetMethodID(clazz, "requestAudioPermission", "()V");

    env->CallVoidMethod(activity, methodId);

    mDefaultCaptureDeviceIndex = 0;
    mDefaultPlaybackDeviceIndex = 0;
}

void Oboe::terminate()
{
}

void Oboe::refreshDevices()
{
    mCaptureDevices.clear();
    mPlaybackDevices.clear();
    
    Device capture(Backend::Oboe);
    capture.name = "Capture device";

    Device playback(Backend::Oboe);
    playback.name = "Playback device";

    mCaptureDevices.push_back(capture);
    mPlaybackDevices.push_back(playback);
}

const std::vector<Device>& Oboe::getCaptureDevices() const
{
    return mCaptureDevices;
}

const std::vector<Device>& Oboe::getPlaybackDevices() const
{
    return mPlaybackDevices;
}

const Device& Oboe::getDefaultCaptureDevice() const
{
    return mCaptureDevices[mDefaultCaptureDeviceIndex];
}

const Device& Oboe::getDefaultPlaybackDevice() const
{
    return mPlaybackDevices[mDefaultPlaybackDeviceIndex];
}

void Oboe::openCaptureStream(const Device *pDevice)
{
    if (pDevice == nullptr) {
        pDevice = &mCaptureDevices[mDefaultCaptureDeviceIndex];
    }

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setChannelCount(oboe::ChannelCount::Mono)
        ->setFormat(oboe::AudioFormat::Float)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setCallback(this)
        ->setUsage(oboe::Usage::Media)
        ->setContentType(oboe::ContentType::Speech)
        ->setInputPreset(oboe::InputPreset::Unprocessed);

    oboe::Result result = builder.openStream(mCaptureStream);
    checkError(result);

    setCaptureBufferSampleRate(mCaptureStream->getSampleRate());
}

void Oboe::startCaptureStream()
{
    oboe::Result result = mCaptureStream->requestStart();
    checkError(result);
}

void Oboe::stopCaptureStream()
{
    oboe::Result result = mCaptureStream->requestStop();
    checkError(result);
}

void Oboe::closeCaptureStream()
{
    oboe::Result result = mCaptureStream->close();
    checkError(result);
    mCaptureStream = nullptr;
}

void Oboe::openPlaybackStream(const Device *pDevice)
{
    if (pDevice == nullptr) {
        pDevice = &mPlaybackDevices[mDefaultPlaybackDeviceIndex];
    }

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
        ->setSharingMode(oboe::SharingMode::Shared)
        ->setChannelCount(oboe::ChannelCount::Mono)
        ->setFormat(oboe::AudioFormat::Float)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setCallback(this)
        ->setUsage(oboe::Usage::Media)
        ->setContentType(oboe::ContentType::Speech);

    oboe::Result result = builder.openStream(mPlaybackStream);
    checkError(result);

    mPlaybackQueue->setOutSampleRate(mPlaybackStream->getSampleRate());
}

void Oboe::startPlaybackStream()
{
    oboe::Result result = mPlaybackStream->requestStart();
    checkError(result);
}

void Oboe::stopPlaybackStream()
{
    oboe::Result result = mPlaybackStream->requestStop();
    checkError(result);
}

void Oboe::closePlaybackStream()
{
    oboe::Result result = mPlaybackStream->close();
    checkError(result);
    mPlaybackStream = nullptr;
}

oboe::DataCallbackResult Oboe::onAudioReady(
        oboe::AudioStream *stream,
        void *data,
        int32_t numFrames)
{
    if (stream->getDirection() == oboe::Direction::Input) {
        // Capture.
        pushToCaptureBuffer(static_cast<const float *>(data), numFrames);
    }
    else if (stream->getDirection() == oboe::Direction::Output) {
        // Playback.
        mPlaybackQueue->pull(static_cast<float *>(data), numFrames);
    }
    return oboe::DataCallbackResult::Continue;
}

void Oboe::onErrorBeforeClose(oboe::AudioStream *stream, oboe::Result result) 
{
    if (result != oboe::Result::ErrorDisconnected) {
        std::cout << "Audio::Oboe] Error before stream close: " << oboe::convertToText(result) << std::endl;
    }

    if (stream->getDirection() == oboe::Direction::Input) {
        // Capture.
    }
    else if (stream->getDirection() == oboe::Direction::Output) {
        // Playback.
    }
}

void Oboe::onErrorAfterClose(oboe::AudioStream *stream, oboe::Result result)
{
    if (stream->getDirection() == oboe::Direction::Input) {
        // Capture.
    }
    else if (stream->getDirection() == oboe::Direction::Output) {
        // Playback.
    }
}

void Oboe::checkError(oboe::Result result)
{
    if (result != oboe::Result::OK) {
        throw std::runtime_error(std::string("Audio::Oboe] ") + oboe::convertToText(result));
    }
}

