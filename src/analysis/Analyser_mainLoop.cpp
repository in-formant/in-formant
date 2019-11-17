//
// Created by rika on 16/11/2019.
//

#include <chrono>
#include "Analyser.h"
#include "../lib/Pitch/Pitch.h"
#include "../lib/LPC/LPC.h"
#include "../lib/LPC/Frame/LPC_Frame.h"
#include "../lib/Signal/Resample.h"

using namespace Eigen;

void Analyser::mainLoop()
{
    using namespace std::chrono;

    static constexpr auto updateDelay = microseconds(static_cast<int>(1'000'000.0 / analysisUpdatesPerSecond));

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    while (running) {

        update();

        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        auto dt = t2 - t1;
        if (dt < updateDelay) {
            std::this_thread::sleep_for(updateDelay - dt);
        }
        else {
            t1 = t2;
        }

    }
}

void Analyser::update()
{
    std::lock_guard<std::mutex> lock(mutex);

    if (!doAnalyse) {
        return;
    }

    // Read captured audio.
    audioCapture.readBlock(x);
    fs = audioCapture.getSampleRate();

    // Shift the audio frames.
    for (int i = 0; i < analysisAudioFrames - 1; ++i) {
        audioFrames[i] = std::move(audioFrames[i + 1]);
    }
    audioFrames[analysisAudioFrames - 1] = x;

    // Get a pitch estimate.
    analysePitch();

    // Resample audio for LP analysis.
    resampleAudio();

    // Perform LP analysis.
    analyseLp();

    // Perform formant analysis with refinement method.
    analyseFormants();

    // Track formants. We'll only look at a small amount of trailing formant frames to save CPU load.
    // The number will be truncated to the largest number of consecutive voiced frames.
    /*
    Formant::Frames tailRawFrames, tailFrames;
    std::copy(rawFormantTrack.frames.end() - tailFormantLength, rawFormantTrack.frames.end(), std::back_inserter(tailRawFrames.frames));

    int maxnFormants = 0;
    for (const auto &frame : tailRawFrames.frames) {
        maxnFormants = std::max(maxnFormants, frame.nFormants);
    }

    Formant::tracker(
            tailRawFrames, tailFrames, maxnFormants, 3,
            550, 1650, 2750, 3850, 4950,
            1.0, 1.0, 1.0);

    std::copy(tailFrames.frames.begin(), tailFrames.frames.end(), formantTrack.frames.end() - tailFormantLength);
    */
}
