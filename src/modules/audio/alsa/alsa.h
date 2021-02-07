#ifndef MODULE_ALSA_H
#define MODULE_ALSA_H

#include "../base/base.h"
#include <alsa/asoundlib.h>
#include <atomic>
#include <thread>
#include <mutex>

namespace Module::Audio {

    class Alsa : public AbstractBase {
    public:
        Alsa();
        ~Alsa();

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

    private:
        void captureThreadLoop();
        void playbackThreadLoop();

        std::mutex mCaptureMutex;
        snd_pcm_t *mCaptureHandle;

        std::mutex mPlaybackMutex;
        snd_pcm_t *mPlaybackHandle;

        std::atomic_bool mThreadsRunning;
        std::thread mCaptureThread;
        std::thread mPlaybackThread;

        rpm::vector<Device> mCaptureDevices;
        rpm::vector<Device> mPlaybackDevices;

        Device mDefaultCaptureDevice;
        Device mDefaultPlaybackDevice;

        bool mPauseCapture;
        bool mPausePlayback;

        int err;
        void checkError();
    };

}

#endif // MODULE_ALSA_H
