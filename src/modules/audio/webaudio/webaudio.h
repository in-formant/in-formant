#ifndef MODULE_WEBAUDIO_H
#define MODULE_WEBAUDIO_H

#include "../base/base.h"
#include <emscripten.h>
#include <emscripten/val.h>

namespace Module::Audio {
    class WebAudio;
}

extern "C" {
void webaudio_process_capture(Module::Audio::WebAudio *self, int32_t framesToProcess, const float *buffer);
void webaudio_process_playback(Module::Audio::WebAudio *self, int32_t framesToProcess, float *buffer);
}

namespace Module::Audio {

    class WebAudio : public AbstractBase {
    public:
        WebAudio();
        ~WebAudio();

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
        int mSampleRate;

        std::vector<Device> mCaptureDevices;
        std::vector<Device> mPlaybackDevices;
    
        Buffer *getBuffer() { return mCaptureBuffer; }
        Queue *getQueue() { return mPlaybackQueue; }

        friend void ::webaudio_process_capture(WebAudio *self, int32_t framesToProcess, const float *buffer);
        friend void ::webaudio_process_playback(WebAudio *self, int32_t framesToProcess, float *buffer);
    };

}

#endif // MODULE_WEBAUDIO_H
