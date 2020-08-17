#include "dummy.h"
#include <iostream>

using namespace Module::Audio;

std::vector<Device> Dummy::captureDevices;
std::vector<Device> Dummy::playbackDevices;

Dummy::Dummy()
{
    Device defInDev(Backend::Dummy);
    defInDev.name = "Dummy input";
    captureDevices = {defInDev};

    Device defOutDev(Backend::Dummy);
    defOutDev.name = "Dummy output";
    playbackDevices = {defOutDev};

    std::fill_n(mDummyInputBuffer.begin(), dummyBufferLength, 0.0f);
    std::fill_n(mDummyOutputBuffer.begin(), dummyBufferLength, 0.0f);
}

Dummy::~Dummy()
{
}

void Dummy::initialize()
{
    std::cout << "Audio::Dummy] initialize ()" << std::endl;
}

void Dummy::terminate()
{
    std::cout << "Audio::Dummy] terminate ()" << std::endl;
}

void Dummy::refreshDevices()
{
    std::cout << "Audio::Dummy] refreshDevices ()" << std::endl;
}

const std::vector<Device>& Dummy::getCaptureDevices() const
{
    return captureDevices;
}

const std::vector<Device>& Dummy::getPlaybackDevices() const
{
    return playbackDevices;
}

const Device& Dummy::getDefaultCaptureDevice() const
{
    return captureDevices[0];
}

const Device& Dummy::getDefaultPlaybackDevice() const
{
    return playbackDevices[0];
}

void Dummy::openCaptureStream(const Device *pDevice) 
{
    std::cout << "Audio::Dummy] openCaptureStream \"" << ((pDevice == nullptr) ? &captureDevices[0] : pDevice)->name << "\"" << std::endl;
}

void Dummy::startCaptureStream()
{
    std::cout << "Audio::Dummy] startCaptureStream ()" << std::endl;
}

void Dummy::stopCaptureStream()
{
    std::cout << "Audio::Dummy] stopCaptureStream ()" << std::endl;
}

void Dummy::closeCaptureStream()
{
    std::cout << "Audio::Dummy] closeCaptureStream ()" << std::endl;
}

void Dummy::openPlaybackStream(const Device *pDevice) 
{
    std::cout << "Audio::Dummy] openPlaybackStream \"" << ((pDevice == nullptr) ? &playbackDevices[0] : pDevice)->name << "\"" << std::endl;
}

void Dummy::startPlaybackStream()
{
    std::cout << "Audio::Dummy] startPlaybackStream ()" << std::endl;
}

void Dummy::stopPlaybackStream()
{
    std::cout << "Audio::Dummy] stopPlaybackStream ()" << std::endl;
}

void Dummy::closePlaybackStream()
{
    std::cout << "Audio::Dummy] closePlaybackStream ()" << std::endl;
}

void Dummy::tickAudio() 
{
    pushToCaptureBuffer(mDummyInputBuffer.data(), dummyBufferLength);
    mPlaybackQueue->pull(mDummyOutputBuffer.data(), dummyBufferLength);
}
