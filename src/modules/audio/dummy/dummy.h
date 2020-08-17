#ifndef MODULE_DUMMY_H
#define MODULE_DUMMY_H

#include "../base/base.h"
#include <array>

namespace Module::Audio {

    class Dummy : public AbstractBase {
    public:
        Dummy();
        ~Dummy();

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

        void tickAudio() override;
        bool needsTicking() override { return true; }

    private:
        static constexpr int dummyBufferLength = 256;
        static std::vector<Device> captureDevices;
        static std::vector<Device> playbackDevices;

        std::array<float, dummyBufferLength> mDummyInputBuffer;
        std::array<float, dummyBufferLength> mDummyOutputBuffer;
    };

}

#endif // MODULE_DUMMY_H
