#ifndef MODULE_PULSE_H
#define MODULE_PULSE_H

#include "../base/base.h"
#include <pulse/pulseaudio.h>

namespace Module::Audio {

    class Pulse : public AbstractBase {
    private:
        static constexpr pa_usec_t apiTimeoutUsec = 5'000'000; // 5 seconds
        static constexpr unsigned long msleepDur = 75;

    public:
        Pulse();
        ~Pulse();

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

    private:
        pa_threaded_mainloop *mThreadedMl;
        pa_mainloop_api *mMlApi;
        pa_context *mContext;

        pa_stream *mCaptureStream;
        pa_stream *mPlaybackStream;
        
        std::vector<Device> mSources;
        std::vector<Device> mSinks;

        Device mDefaultSource;
        Device mDefaultSink;

        bool mPauseCapture;
        bool mPausePlayback;

        void checkError();
    };

}

#endif // MODULE_PULSE_H
