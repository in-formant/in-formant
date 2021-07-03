#include "alsa.h"
#include <iostream>
#include <memory>

using namespace Module::Audio;

Alsa::Alsa()
    : mDefaultCaptureDevice(Backend::ALSA),
      mDefaultPlaybackDevice(Backend::ALSA),
      mPauseCapture(true),
      mPausePlayback(true)
{
    mDefaultCaptureDevice.name = "Default input";
    mDefaultCaptureDevice.alsa.hintName = strdup("default");

    mDefaultPlaybackDevice.name = "Default output";
    mDefaultPlaybackDevice.alsa.hintName = strdup("default");
}

Alsa::~Alsa()
{
}

void Alsa::initialize()
{
    mThreadsRunning = true;
    mCaptureThread = std::thread(std::mem_fn(&Alsa::captureThreadLoop), this);
    mPlaybackThread = std::thread(std::mem_fn(&Alsa::playbackThreadLoop), this);
}

void Alsa::terminate()
{
    mThreadsRunning = false;
    mCaptureThread.join();
    mPlaybackThread.join();
}

void Alsa::refreshDevices()
{
    void **hints;
    err = snd_device_name_hint(-1, "pcm", &hints);
    checkError();
    
    void **cur = hints;

    while (*cur != nullptr) {
        char *name = snd_device_name_get_hint(*cur, "NAME");
        char *desc = snd_device_name_get_hint(*cur, "DESC");
        char *ioid = snd_device_name_get_hint(*cur, "IOID");

        Device dev(Backend::ALSA);

        if (desc != nullptr) {
            if (char *pch = strchr(desc, '\n'); pch != nullptr) {
                *pch = '\0';
            }
            dev.name = desc;
            free(desc);
        }
        else {
            // If we couldn't find a DESC hint, fallback to using the alsa name.
            dev.name = name;
        }

        // Freeing this is handled by the Device destructor.
        dev.alsa.hintName = name;

        bool isInput(false), isOutput(false);

        if (ioid != nullptr) {
            if (strcmp(ioid, "Input") == 0) {
                isInput = true;
            }
            else if (strcmp(ioid, "Output") == 0) {
                isOutput = true;
            }
            free(ioid);
        }
        else {
            isInput = isOutput = true;
        }

        if (isInput) {
            mCaptureDevices.push_back(dev);
        }
        if (isOutput) {
            mPlaybackDevices.push_back(dev);
        }
    
        cur++;
    }

    snd_device_name_free_hint(hints);
}

const rpm::vector<Device>& Alsa::getCaptureDevices() const
{
    return mCaptureDevices;
}

const rpm::vector<Device>& Alsa::getPlaybackDevices() const
{
    return mPlaybackDevices;
}

const Device& Alsa::getDefaultCaptureDevice() const
{
    return mDefaultCaptureDevice;
}

const Device& Alsa::getDefaultPlaybackDevice() const
{
    return mDefaultPlaybackDevice;
}

void Alsa::openCaptureStream(const Device *pDevice) 
{
    if (pDevice == nullptr) {
        pDevice = &mDefaultCaptureDevice;
    }
   
    mPauseCapture = true;

    err = snd_pcm_open(&mCaptureHandle, pDevice->alsa.hintName, SND_PCM_STREAM_CAPTURE, 0);
    checkError();

    err = snd_pcm_set_params(
            mCaptureHandle,
            SND_PCM_FORMAT_FLOAT,
            SND_PCM_ACCESS_RW_INTERLEAVED,
            1,
            48000,
            true,
            50000);
    checkError();

    setCaptureBufferSampleRate(48000);
}

void Alsa::startCaptureStream()
{
    snd_pcm_prepare(mCaptureHandle);
    mPauseCapture = false;
    snd_pcm_start(mCaptureHandle);
}

void Alsa::stopCaptureStream()
{
    mPauseCapture = true;
}

void Alsa::closeCaptureStream()
{
    mPauseCapture = true;
    
    snd_pcm_close(mCaptureHandle);
}

void Alsa::openPlaybackStream(const Device *pDevice) 
{
    if (pDevice == nullptr) {
        pDevice = &mDefaultPlaybackDevice;
    }

    mPausePlayback = true;

    err = snd_pcm_open(&mPlaybackHandle, pDevice->alsa.hintName, SND_PCM_STREAM_PLAYBACK, 0);
    checkError();

    err = snd_pcm_set_params(
            mPlaybackHandle,
            SND_PCM_FORMAT_FLOAT,
            SND_PCM_ACCESS_RW_INTERLEAVED,
            1,
            48000,
            true,
            50000);
    checkError();
        
    mPlaybackQueue->setOutSampleRate(48000);
}

void Alsa::startPlaybackStream()
{
    snd_pcm_prepare(mPlaybackHandle);
    mPausePlayback = false;
    snd_pcm_start(mPlaybackHandle);
}

void Alsa::stopPlaybackStream()
{
    mPausePlayback = true;
}

void Alsa::closePlaybackStream()
{
    mPausePlayback = true;

    snd_pcm_close(mPlaybackHandle);
}

void Alsa::captureThreadLoop()
{
    rpm::vector<float> array;

    while (mThreadsRunning) {
        while (mThreadsRunning && mPauseCapture)
            usleep(50'000);
    
        if (!mThreadsRunning)
            break;
        
        int frameCount = snd_pcm_avail(mCaptureHandle);
        if (frameCount < 0) {
            std::cout << "Audio::ALSA] Capture stream query failed: " << snd_strerror(frameCount) << std::endl;
        }
        else if (frameCount > 0) {
            array.resize(frameCount);

            int ret = snd_pcm_readi(mCaptureHandle, array.data(), frameCount);
            if (ret < 0)
                ret = snd_pcm_recover(mCaptureHandle, ret, true);
            if (ret < 0) {
                std::cout << "Audio::ALSA] Capture stream read failed: " << snd_strerror(ret) << std::endl;
            }

            if (ret > 0 && ret < frameCount) {
                std::cout << "Audio::ALSA] Capture stream read fewer frames than expected" << std::endl;
            }

            pushToCaptureBuffer(array.data(), ret);
        }
    }
}

void Alsa::playbackThreadLoop()
{
    rpm::vector<float> array;

    while (mThreadsRunning) {
        while (mThreadsRunning && mPausePlayback)
            usleep(50'000);
    
        if (!mThreadsRunning)
            break;
        
        int frameCount = snd_pcm_avail(mPlaybackHandle);
        if (frameCount < 0) {
            std::cout << "Audio::ALSA] Playback stream query failed: " << snd_strerror(frameCount) << std::endl;
        }
        else {
            array.resize(frameCount);

            mPlaybackQueue->pull(array.data(), frameCount);

            int ret = snd_pcm_writei(mPlaybackHandle, array.data(), frameCount);
            if (ret < 0)
                ret = snd_pcm_recover(mPlaybackHandle, ret, true);
            if (ret < 0) {
                std::cout << "Audio::ALSA] Playback stream write failed: " << snd_strerror(ret) << std::endl;
            }

            if (ret > 0 && ret < frameCount) {
                std::cout << "Audio::ALSA] Playback stream wrote fewer frames than expected" << std::endl;
            }
        }
    }
}

void Alsa::checkError()
{
    if (err != 0) {
        throw std::runtime_error(std::string("Audio::ALSA] ") + snd_strerror(err));
    }
}
