#ifndef MAIN_AUDIO_CONTEXT_H
#define MAIN_AUDIO_CONTEXT_H

#include "../modules/audio/audio.h"

namespace Main {

    using namespace Module;

    Audio::Backend getDefaultAudioBackend();
    std::vector<Audio::Backend> getSupportedAudioBackends();
    std::string getAudioBackendName(Audio::Backend type);

    class AudioContext {
    public:
        AudioContext(Audio::Backend type, Audio::Buffer *captureBuffer, Audio::Queue *playbackQueue);
        virtual ~AudioContext();

        void refreshDevices();

        const std::vector<Audio::Device>& getCaptureDevices() const;
        const std::vector<Audio::Device>& getPlaybackDevices() const;
        
        const Audio::Device& getDefaultCaptureDevice() const;
        const Audio::Device& getDefaultPlaybackDevice() const;

        void openCaptureStream(const Audio::Device *pDevice);
        void startCaptureStream();
        void stopCaptureStream();
        void closeCaptureStream();

        void openPlaybackStream(const Audio::Device *pDevice);
        void startPlaybackStream();
        void stopPlaybackStream();
        void closePlaybackStream();

        void tickAudio();

    private:
        std::unique_ptr<Audio::AbstractBase> mAudio;
        bool mCaptureStreamOpened, mCaptureStreamStarted;
        bool mPlaybackStreamOpened, mPlaybackStreamStarted;
    };

}

#endif // MAIN_AUDIO_CONTEXT_H
