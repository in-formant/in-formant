#ifndef AUDIO_BASE_H
#define AUDIO_BASE_H

#include "rpcxx.h"
#include "../buffer/buffer.h"
#include "../queue/queue.h"
#include <initializer_list>
#include <string>
#include <cstring>

#ifdef AUDIO_USE_PORTAUDIO
#   include <portaudio.h>
#endif

namespace Module::Audio {

    struct dev_dummy_t {
#ifdef AUDIO_USE_DUMMY
        // Nothing.
#endif
    };

    struct dev_alsa_t {
#ifdef AUDIO_USE_ALSA
        // The dev.name field will contain the first line of the DESC hint.
        // This contains the NAME hint. It will be used to identify a device internally.
        char *hintName;
#endif
    };

    struct dev_pulse_t {
#ifdef AUDIO_USE_PULSE
        // Nothing.
#endif
    };

    struct dev_portaudio_t {
#ifdef AUDIO_USE_PORTAUDIO
        PaDeviceIndex index;
        PaTime inputLatency;
        PaTime outputLatency;
        double sampleRate;
#endif
    };

    struct dev_oboe_t {
#ifdef AUDIO_USE_OBOE
        // Nothing.
#endif
    };
    
    struct dev_webaudio_t {
#ifdef AUDIO_USE_WEBAUDIO
        // Nothing.
#endif
    };

    enum class Backend : uint64_t {
        Dummy,
        ALSA,
        Pulse,
        PortAudio,
        Oboe,
        WebAudio,
    };

    struct Device {
        Device(Backend b) : backend(b) {}
        ~Device() {
#ifdef AUDIO_USE_ALSA
            if (backend == Backend::ALSA) {
                if (!__builtin_constant_p(alsa.hintName)) {
                    free(alsa.hintName);
                }
            }
#endif
        }

        Device(const Device& o) : backend(o.backend), name(o.name) {
            switch (backend) {
            case Backend::Dummy:
                dummy = o.dummy;
                break;
            case Backend::ALSA:
                alsa = o.alsa;
#ifdef AUDIO_USE_ALSA
                alsa.hintName = strdup(o.alsa.hintName);
#endif
                break;
            case Backend::Pulse:
                pulse = o.pulse;
                break;
            case Backend::PortAudio:
                portaudio = o.portaudio;
                break;
            case Backend::Oboe:
                oboe = o.oboe;
                break;
            case Backend::WebAudio:
                webaudio = o.webaudio;
                break;
            }
        }

        Backend backend;
        std::string name;
        union {
            dev_dummy_t     dummy;
            dev_alsa_t      alsa;
            dev_pulse_t     pulse;
            dev_portaudio_t portaudio;
            dev_oboe_t      oboe;
            dev_webaudio_t  webaudio;
        };
    };

    class AbstractBase {
    public:
        AbstractBase();
        virtual ~AbstractBase();

        virtual void initialize() = 0;
        virtual void terminate() = 0;
   
        virtual void refreshDevices() = 0;

        virtual const rpm::vector<Device>& getCaptureDevices() const = 0;
        virtual const rpm::vector<Device>& getPlaybackDevices() const = 0;
        
        virtual const Device& getDefaultCaptureDevice() const = 0;
        virtual const Device& getDefaultPlaybackDevice() const = 0;

        virtual void openCaptureStream(const Device *pDevice) = 0;
        virtual void startCaptureStream() = 0;
        virtual void stopCaptureStream() = 0;
        virtual void closeCaptureStream() = 0;

        virtual void openPlaybackStream(const Device *pDevice) = 0;
        virtual void startPlaybackStream() = 0;
        virtual void stopPlaybackStream() = 0;
        virtual void closePlaybackStream() = 0;

        virtual void tickAudio();
        virtual bool needsTicking() = 0;

        void setCaptureBuffer(Buffer *buffer);
        void setPlaybackQueue(Queue *queue);

    protected:
        void setCaptureBufferSampleRate(int sampleRate);
        void pushToCaptureBuffer(const float *data, int length);

        Buffer *mCaptureBuffer;
        Queue *mPlaybackQueue;
    };

}

#endif // AUDIO_BASE_H
