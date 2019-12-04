//
// Created by rika on 16/11/2019.
//

#include <chrono>
#include "Analyser.h"
#include "../lib/Pitch/Pitch.h"
#include "../lib/Signal/Filter.h"
#include "../lib/Signal/Window.h"

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

    // Get a pitch estimate.
    analysePitch();

    // Resample audio for LP analysis.
    resampleAudio();

    // Pre-emphasis.
    preEmphGauss();

    // Perform LP analysis.
    analyseLp();

    // Perform formant analysis from LP coefficients.
    analyseFormantLp();

}
