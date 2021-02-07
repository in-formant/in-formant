#ifndef MODULE_PORTAUDIO_H
#define MODULE_PORTAUDIO_H

#include "../base/base.h"
#include <portaudio.h>

namespace Module::Audio {

    class PortAudio : public AbstractBase {
    public:
        PortAudio();
        ~PortAudio();

        void initialize() override;
        void terminate() override;

        void refreshDevices() override;

        const rpm::vector<Device>& getCaptureDevices() const override;
        const rpm::vector<Device>& getPlaybackDevices() const override;

        const Device& getDefaultCaptureDevice() const override;
        const Device& getDefaultPlaybackDevice() const override;

        void openCaptureStream(const Device *pDevice) override;
        void startCaptureStream() override;
        void stopCaptureStream() override;
        void closeCaptureStream() override;

        void openPlaybackStream(const Device *pDevice) override;
        void startPlaybackStream() override;
        void stopPlaybackStream() override;
        void closePlaybackStream() override;

        bool needsTicking() override { return false; }

        static int captureCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
        static int playbackCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

    private:
        PaStream *mCaptureStream;
        PaStream *mPlaybackStream;

        rpm::vector<Device> mCaptureDevices;
        rpm::vector<Device> mPlaybackDevices;

        int mDefaultCaptureDeviceIndex;
        int mDefaultPlaybackDeviceIndex;

        bool mPauseCapture;
        bool mPausePlayback;

        PaError err;
        void checkError();
    };

}

#endif // MODULE_PORTAUDIO_H
