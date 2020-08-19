#ifndef MODULE_OBOE_H
#define MODULE_OBOE_H

#include "../base/base.h"
#include <oboe/Oboe.h>
#include <memory>

namespace Module::Audio {

    class Oboe : public AbstractBase, oboe::AudioStreamCallback {
    public:
        Oboe();
        ~Oboe();

        void initialize() override;
        void terminate() override;

        void refreshDevices() override;

        const std::vector<Device>& getCaptureDevices() const override;
        const std::vector<Device>& getPlaybackDevices() const override;

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

        oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *data, int32_t numFrames) override;
        void onErrorBeforeClose(oboe::AudioStream *stream, oboe::Result result) override;
        void onErrorAfterClose(oboe::AudioStream *stream, oboe::Result result) override;

    private:
        std::shared_ptr<oboe::AudioStream> mCaptureStream;
        std::shared_ptr<oboe::AudioStream> mPlaybackStream;

        std::vector<Device> mCaptureDevices;
        std::vector<Device> mPlaybackDevices;

        int mDefaultCaptureDeviceIndex;
        int mDefaultPlaybackDeviceIndex;

        bool mPauseCapture;
        bool mPausePlayback;

        void checkError(oboe::Result result);
    };

}

#endif // MODULE_OBOE_H
