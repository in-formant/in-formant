#include "audiocontext.h"
#include <stdexcept>

using namespace Main;

static std::vector<Audio::Backend> supportedAudioBackends = {
#ifdef AUDIO_USE_DUMMY
    Audio::Backend::Dummy,
#endif
#ifdef AUDIO_USE_ALSA
    Audio::Backend::ALSA,
#endif
#ifdef AUDIO_USE_PULSE
    Audio::Backend::Pulse,
#endif
#ifdef AUDIO_USE_PORTAUDIO
    Audio::Backend::PortAudio,
#endif
#ifdef AUDIO_USE_OBOE
    Audio::Backend::Oboe,
#endif
#ifdef AUDIO_USE_WEBAUDIO
    Audio::Backend::WebAudio,
#endif
};

Audio::Backend Main::getDefaultAudioBackend()
{
#if defined(_WIN32) || defined(__APPLE__)
    return Audio::Backend::PortAudio;
#elif defined(__linux)
    return Audio::Backend::ALSA;
#elif defined(ANDROID) || defined(__ANDROID__)
    return Audio::Backend::Oboe;
#elif defined(__EMSCRIPTEN__)
    return Audio::Backend::WebAudio;
#else
#error "No default backend for this platform."
#endif
}

std::vector<Audio::Backend> Main::getSupportedAudioBackends()
{
    return supportedAudioBackends;
}

std::string Main::getAudioBackendName(Audio::Backend type)
{
    switch (type) {
    case Audio::Backend::Dummy:
        return "Dummy";
    case Audio::Backend::ALSA:
        return "ALSA";
    case Audio::Backend::Pulse:
        return "PulseAudio";
    case Audio::Backend::PortAudio:
        return "PortAudio";
    case Audio::Backend::Oboe:
        return "Oboe";
    case Audio::Backend::WebAudio:
        return "WebAudio";
    default:
        return "Unknown";
    }
}

AudioContext::AudioContext(Audio::Backend type, Audio::Buffer *captureBuffer, Audio::Queue *playbackQueue)
    : mCaptureStreamOpened(false),
      mCaptureStreamStarted(false),
      mPlaybackStreamOpened(false),
      mPlaybackStreamStarted(false)
{    
    switch (type) {
#ifdef AUDIO_USE_DUMMY
    case Audio::Backend::Dummy:
        mAudio = std::make_unique<Audio::Dummy>();
        break;
#endif
#ifdef AUDIO_USE_ALSA
    case Audio::Backend::ALSA:
        mAudio = std::make_unique<Audio::Alsa>();
        break;
#endif
#ifdef AUDIO_USE_PULSE
    case Audio::Backend::Pulse:
        mAudio = std::make_unique<Audio::Pulse>();
        break;
#endif
#ifdef AUDIO_USE_PORTAUDIO
    case Audio::Backend::PortAudio:
        mAudio = std::make_unique<Audio::PortAudio>();
        break;
#endif
#ifdef AUDIO_USE_OBOE
    case Audio::Backend::Oboe:
        mAudio = std::make_unique<Audio::Oboe>();
        break;
#endif
#ifdef AUDIO_USE_WEBAUDIO
    case Audio::Backend::WebAudio:
        mAudio = std::make_unique<Audio::WebAudio>();
        break;
#endif
    default:
        break;
    }

    if (!mAudio) {
        throw std::runtime_error("AudioContext] Backend could not be initialized.");
    }

    mAudio->setCaptureBuffer(captureBuffer);
    mAudio->setPlaybackQueue(playbackQueue);
    mAudio->initialize();
    mAudio->refreshDevices();
}

AudioContext::~AudioContext()
{
    stopCaptureStream();
    closeCaptureStream();
    stopPlaybackStream();
    closePlaybackStream();
    mAudio->terminate();
}

void AudioContext::refreshDevices()
{
    mAudio->refreshDevices();
}

const std::vector<Audio::Device>& AudioContext::getCaptureDevices() const
{
    return mAudio->getCaptureDevices();
}

const std::vector<Audio::Device>& AudioContext::getPlaybackDevices() const
{
    return mAudio->getPlaybackDevices();
}
 
const Audio::Device& AudioContext::getDefaultCaptureDevice() const
{
    return mAudio->getDefaultCaptureDevice();
}

const Audio::Device& AudioContext::getDefaultPlaybackDevice() const
{
    return mAudio->getDefaultPlaybackDevice();
}

void AudioContext::openCaptureStream(const Audio::Device *pDevice)
{
    if (!mCaptureStreamOpened) {
        mAudio->openCaptureStream(pDevice);
        mCaptureStreamOpened = true;
    }
}

void AudioContext::startCaptureStream()
{
    if (!mCaptureStreamStarted) {
        mAudio->startCaptureStream();
        mCaptureStreamStarted = true;
    }
}

void AudioContext::stopCaptureStream()
{
    if (mCaptureStreamStarted) {
        mAudio->stopCaptureStream();
        mCaptureStreamStarted = false;
    }
}

void AudioContext::closeCaptureStream()
{
    if (mCaptureStreamOpened) {
        mAudio->closeCaptureStream();
        mCaptureStreamOpened = false;
    }
}

void AudioContext::openPlaybackStream(const Audio::Device *pDevice)
{
    if (!mPlaybackStreamOpened) {
        mAudio->openPlaybackStream(pDevice);
        mPlaybackStreamOpened = true;
    }
}

void AudioContext::startPlaybackStream()
{
    if (!mPlaybackStreamStarted) {
        mAudio->startPlaybackStream();
        mPlaybackStreamStarted = true;
    }
}

void AudioContext::stopPlaybackStream()
{
    if (mPlaybackStreamStarted) {
        mAudio->stopPlaybackStream();
        mPlaybackStreamStarted = false;
    }
}

void AudioContext::closePlaybackStream()
{
    if (mPlaybackStreamOpened) {
        mAudio->closePlaybackStream();
        mPlaybackStreamOpened = false;
    }
}

void AudioContext::tickAudio()
{
    if (mAudio->needsTicking()) {
        mAudio->tickAudio();
    }
}

