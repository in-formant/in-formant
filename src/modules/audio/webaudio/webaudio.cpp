#include "webaudio.h"

using namespace Module::Audio;

WebAudio::WebAudio()
{
}

WebAudio::~WebAudio()
{
}

void WebAudio::initialize()
{
    int resultFromJs = EM_ASM_INT({
        if ((window.AudioContext || window.webkitAudioContext) === undefined) {
            return 0;
        }

        if (typeof(_webaudio) === 'undefined') {
            _webaudio = {};
        }

        return 1;
    }, 0);

    if (resultFromJs == 0) {
        throw std::runtime_error("Audio::WebAudio] WebAudio not supported.");
    }

    mSampleRate = EM_ASM_INT({
        try {
            const temp = new (window.AudioContext || window.webkitAudioContext)();
            const sampleRate = temp.sampleRate;
            temp.close();
            return sampleRate;
        } 
        catch (e) {
            return 0;
        }
    }, 0);
    
    if (mSampleRate == 0) {
        throw std::runtime_error("Audio::WebAudio] No device.");
    }
}

void WebAudio::terminate()
{
    EM_ASM({
        Module.free(_webaudio.captureDevice.buffer);
        Module.free(_webaudio.playbackDevice.buffer);
    });
}

void WebAudio::refreshDevices()
{
    mCaptureDevices.clear();
    mPlaybackDevices.clear();
    
    Device capture(Backend::Oboe);
    capture.name = "Capture device";

    Device playback(Backend::Oboe);
    playback.name = "Playback device";

    mCaptureDevices.push_back(capture);
    mPlaybackDevices.push_back(playback);
}

const std::vector<Device>& WebAudio::getCaptureDevices() const
{
    return mCaptureDevices;
}

const std::vector<Device>& WebAudio::getPlaybackDevices() const
{
    return mPlaybackDevices;
}

const Device& WebAudio::getDefaultCaptureDevice() const
{
    return mCaptureDevices[0];
}

const Device& WebAudio::getDefaultPlaybackDevice() const
{
    return mPlaybackDevices[0];
}

void WebAudio::openCaptureStream(const Device *pDevice)
{
    (void) pDevice;

    const int channelCount = 1;
    const int bufferSize = 512;

    EM_ASM({
        var channels   = $0;
        var sampleRate = $1;
        var bufferSize = $2;
        var pThis      = $3;
        
        var device = {};

        device.context = new (window.AudioContext || window.webkitAudioContext)({sampleRate:sampleRate});
        device.context.suspend();

        device.bufferSize = channels * bufferSize * 4;
        device.buffer = Module._malloc(device.bufferSize);
        device.bufferView = new Float32Array(Module.HEAPF32.buffer, device.buffer, device.bufferSize);

        device.scriptNode = device.context.createScriptProcessor(bufferSize, channels, channels);

        device.scriptNode.onaudioprocess = function(e) {
            if (device.buffer === undefined) {
                return;
            }

            for (var ch = 0; ch < e.outputBuffer.numberOfChannels; ++ch) {
                e.outputBuffer.getChannelData(ch).fill(0.0);
            }

            var sendSilence = device.silenced || (device.streamNode === undefined);

            var totalFramesProcessed = 0;
            while (totalFramesProcessed < e.inputBuffer.length) {
                var framesRemaining = e.inputBuffer.length - totalFramesProcessed;
                var framesToProcess = framesRemaining;
                if (framesToProcess > (device.bufferSize / channels / 4)) {
                    framesToProcess = (device.bufferSize / channels / 4);
                }

                if (sendSilence) {
                    device.bufferView.fill(0.0);
                }
                else {
                    for (var i = 0; i < framesToProcess; ++i) {
                        for (var ch = 0; ch < e.inputBuffer.numberOfChannels; ++ch) {
                            device.bufferView[i * channels + ch] =
                                e.inputBuffer.getChannelData(ch)[totalFramesProcessed + i];
                        }
                    }
                }

                ccall("webaudio_process_capture", "undefined", ["number", "number", "number"], [pThis, framesToProcess, device.buffer]);

                totalFramesProcessed += framesToProcess;
            }
        };

        navigator.mediaDevices.getUserMedia({audio:true, video:false})
            .then(function(stream) {
                device.streamNode = device.context.createMediaStreamSource(stream);
                device.streamNode.connect(device.scriptNode);
                device.scriptNode.connect(device.context.destination);
            })
            .catch(function(error) {
                device.scriptNode.connect(device.context.destination);
            });

        device.silenced = true;
        _webaudio.captureDevice = device;
    }, channelCount, mSampleRate, bufferSize, this);
}

void WebAudio::startCaptureStream()
{
    EM_ASM({
        _webaudio.captureDevice.context.resume();
        _webaudio.captureDevice.silenced = false;
    });
}

void WebAudio::stopCaptureStream()
{
    EM_ASM({
        _webaudio.captureDevice.silenced = true;
        _webaudio.captureDevice.context.suspend();
    });
}

void WebAudio::closeCaptureStream()
{
    EM_ASM({
        _webaudio.captureDevice.context.close();
    });
}

void WebAudio::openPlaybackStream(const Device *pDevice)
{
    (void) pDevice;

    const int channelCount = 1;
    const int bufferSize = 512;

    EM_ASM({
        var channels   = $0;
        var sampleRate = $1;
        var bufferSize = $2;
        var pThis      = $3;
        
        var device = {};

        device.context = new (window.AudioContext || window.webkitAudioContext)({sampleRate:sampleRate});
        device.context.suspend();
        
        device.bufferSize = channels * bufferSize * 4;
        device.buffer = Module._malloc(device.bufferSize);
        device.bufferView = new Float32Array(Module.HEAPF32.buffer, device.buffer, device.bufferSize);

        device.scriptNode = device.context.createScriptProcessor(bufferSize, channels, channels);

        device.scriptNode.onaudioprocess = function(e) {
            if (device.buffer === undefined) {
                return;
            }

            var sendSilence = device.silenced || (e.outputBuffer.numberOfChannels != channels);

            var totalFramesProcessed = 0;
            while (totalFramesProcessed < e.outputBuffer.length) {
                var framesRemaining = e.outputBuffer.length - totalFramesProcessed;
                var framesToProcess = framesRemaining;
                if (framesToProcess > (device.bufferSize / channels / 4)) {
                    framesToProcess = (device.bufferSize / channels / 4);
                }

                ccall("webaudio_process_playback", "undefined", ["number", "number", "number"], [pThis, framesToProcess, device.buffer]);

                if (sendSilence) {
                    for (var ch = 0; ch < e.outputBuffer.numberOfChannels; ++ch) {
                        e.outputBuffer.getChannelData(ch).fill(0.0);
                    }
                }
                else {
                    for (var ch = 0; ch < e.outputBuffer.numberOfChannels; ++ch) {
                        for (var i = 0; i < framesToProcess; ++i) {
                            e.outputBuffer.getChannelData(ch)[totalFramesProcessed + i] =
                                device.bufferView[i * channels + ch];
                        }
                    }
                }

                totalFramesProcessed += framesToProcess;
            }
        };
        
        device.scriptNode.connect(device.context.destination);

        device.silenced = true;
        _webaudio.playbackDevice = device;
    }, channelCount, mSampleRate, bufferSize, this);
}

void WebAudio::startPlaybackStream()
{
    EM_ASM({
        _webaudio.playbackDevice.context.resume();
        _webaudio.playbackDevice.silenced = false;
    });
}

void WebAudio::stopPlaybackStream()
{
    EM_ASM({
        _webaudio.playbackDevice.silenced = true;
        _webaudio.playbackDevice.context.suspend();
    });
}

void WebAudio::closePlaybackStream()
{
    EM_ASM({
        _webaudio.playbackDevice.context.close();
    });
}

void webaudio_process_capture(WebAudio *self, int32_t framesToProcess, const float *buffer)
{
    self->mCaptureBuffer->push(buffer, framesToProcess);
}

void webaudio_process_playback(WebAudio *self, int32_t framesToProcess, float *buffer)
{
    self->mPlaybackQueue->pull(buffer, framesToProcess);
}
